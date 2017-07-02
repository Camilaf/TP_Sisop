// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW 0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.

	pte_t pte = uvpt[PGNUM(addr)];
	// (1)
	if (!(err & 2))
		panic("pgfault: the faulting access was a read!");
		
	// (2)
	if (!(pte & PTE_COW))
		panic("pgfault: it is not a copy-on-write page!");


	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.

	//panic("pgfault not implemented");
	addr = ROUNDDOWN(addr, PGSIZE);
	
	if ((r = sys_page_alloc(sys_getenvid(), (void *) PFTEMP, PTE_U | PTE_P | PTE_W)) < 0)
		panic("pgfault: sys_page_alloc: %e", r);
	memmove(PFTEMP, addr, PGSIZE);
	
	if ((r = sys_page_map(sys_getenvid(), PFTEMP, sys_getenvid(), addr, PTE_U | PTE_P | PTE_W)) < 0)
		panic("sys_page_map: %e", r);
	
	if ((r = sys_page_unmap(sys_getenvid(), PFTEMP)) < 0)
		panic("sys_page_unmap: %e", r);
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
	//panic("duppage not implemented");
	pte_t pte = uvpt[pn];
	
	if ((pte & PTE_W) || (pte & PTE_COW)) {
		if ((r = sys_page_map(sys_getenvid(), (void *) (pn*PGSIZE), envid, (void *) (pn*PGSIZE), PTE_U | PTE_P | PTE_COW)) < 0) 
			return r;
		
		// Se hace el remap de la copy-on-write page en su propio espacio de direcciones
		if ((r = sys_page_map(sys_getenvid(), (void *) (pn*PGSIZE), sys_getenvid(), (void *) (pn*PGSIZE), PTE_U | PTE_P | PTE_COW)) < 0) 
			return r;
	
	}
	// Not writable or copy-on-write pages
	else {
		if ((r = sys_page_map(sys_getenvid(), (void *) (pn*PGSIZE), envid, (void *) (pn*PGSIZE), PTE_P | PTE_U)) < 0) 
			return r;
	}
	
	return 0;
}

static void
dup_or_share(envid_t dstenv, void *va, int perm) {
	//panic("dup_or_share not implemented");
	int r;
	
	if (perm & PTE_MAPPED) {
		if ((r = sys_page_map(0, va, dstenv, va, perm)) < 0)
			panic("sys_page_map: %e", r);
	}
	// Read-only case
	else if (!(perm & PTE_W)) {
		if ((r = sys_page_map(0, va, dstenv, va, perm & PTE_SYSCALL)) < 0)
			panic("sys_page_map: %e", r);
	}
	else {
		if ((r = sys_page_alloc(dstenv, va, perm & PTE_SYSCALL)) < 0)
			panic("sys_page_alloc: %e", r);
		if ((r = sys_page_map(dstenv, va, 0, UTEMP, perm & PTE_SYSCALL)) < 0)
			panic("sys_page_map: %e", r);
		memmove(UTEMP, va, PGSIZE);
		if ((r = sys_page_unmap(0, UTEMP)) < 0)
			panic("sys_page_unmap: %e", r);
	}
}

envid_t
fork_v0(void) {
	//panic("fork_v0 not implemented");
	
	envid_t envid;
	uint8_t *addr;
	int r;

	// Allocate a new child environment.
	// The kernel will initialize it with a copy of our register state,
	// so that the child will appear to have called sys_exofork() too -
	// except that in the child, this "fake" call to sys_exofork()
	// will return 0 instead of the envid of the child.
	envid = sys_exofork();
	if (envid < 0)
		panic("sys_exofork: %e", envid);
	if (envid == 0) {
		// We're the child.
		// The copied value of the global variable 'thisenv'
		// is no longer valid (it refers to the parent!).
		// Fix it and return 0.
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}
	
	// We're the parent.
	
	for (addr = 0; addr < (uint8_t*) UTOP; addr += PGSIZE) {
		if (uvpd[PDX(addr)] & PTE_P) {
			pte_t pte = uvpt[PGNUM(addr)];
			if (pte & PTE_P) //Si la page esta mapeada
				dup_or_share(envid, addr, pte & 0xFFF);
		}
	}
	
	// Start the child environment running
	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0)
		panic("sys_env_set_status: %e", r);

	return envid;
}


//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	//panic("fork not implemented"); 
	
	// Fork_v0
	//return fork_v0();
	
	// Copy-on-Write Fork
	envid_t envid;
	uint8_t *addr;
	int r;
	
	set_pgfault_handler(pgfault);
	envid = sys_exofork();
	if (envid < 0)
		panic("sys_exofork: %e", envid);
	// Child
	if (envid == 0) {
		// We're the child.
		// The copied value of the global variable 'thisenv'
		// is no longer valid (it refers to the parent!).
		// Fix it and return 0.
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}
	
	// Parent
	for (addr = 0; addr < (uint8_t*) (UXSTACKTOP - PGSIZE); addr += PGSIZE) {
		if (uvpd[PDX(addr)] & PTE_P) {
			pte_t pte = uvpt[PGNUM(addr)];
			if ((pte & PTE_W) || (pte & PTE_COW)) {
				r = duppage(envid, PGNUM(addr));
				if (r < 0)
					panic("fork: duppage: %e", r);		
			}
		}
	}
	
	// Allocate a fresh page in the child for the exception stack
	sys_page_alloc(envid, (void *) (UXSTACKTOP - PGSIZE), PTE_P | PTE_U | PTE_W);
	
	// Set the user page fault entrypoint for the child
	int err = sys_env_set_pgfault_upcall(envid, thisenv->env_pgfault_upcall);
	if (err < 0)
		panic("fork: sys_env_set_pgfault_upcall: %e", err);
	
	// Start the child environment running
	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0)
		panic("sys_env_set_status: %e", r);

	return envid;
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}

