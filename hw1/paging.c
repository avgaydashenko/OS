#include "paging.h"

pte_t *pml4;

void map_init() {
    pml4 = get_mem(PAGE_SIZE, PAGE_SIZE);

    for (phys_t i = 0;
         i < memory_map[memory_map_size - 1].address + memory_map[memory_map_size - 1].length;
         i += (1 << 21)) {
        
        map_address(i + HIGH_BASE, i, USE_BIG_PAGE | USE_BOOT_ALLOCATE | NOT_FLUSH_TLB);
        if (i + KERNEL_BASE > HIGH_BASE) {
            map_address(i + KERNEL_BASE, i, USE_BIG_PAGE | USE_BOOT_ALLOCATE | NOT_FLUSH_TLB);
        }
    }

    store_pml4(pa(pml4));

    serial_port_write_line("Initialise page mapping: successful.\n");
}

void force_pte(pte_t *pmle, int flags) {
    if (!pte_present(*pmle)) {
        void *npg;
        if (flags & USE_BOOT_ALLOCATE) {
            npg = get_mem(PAGE_SIZE, PAGE_SIZE);
        } else {
            npg = get_page(0);
                for (size_t i = 0; i < PAGE_SIZE/sizeof(npg[0]); ++i) {
                    (& npg)[i] = 0;
                }           
        }

        *pmle = pa(npg) | PTE_PRESENT | PTE_WRITE;
    }
}

void map_address(virt_t vad, phys_t pad, int flags) {

    pte_t *pml4e = pml4 + pml4_i(vad);

    force_pte(pml4e, flags);

    pte_t *pdpte = ((pte_t *) va(pte_phys(*pml4e) << 12)) + pml3_i(vad);

    force_pte(pdpte, flags);

    pte_t *pde = ((pte_t *) va(pte_phys(*pdpte) << 12)) + pml2_i(vad);
    if (flags & USE_BIG_PAGE) {
        *pde = pad | PTE_PRESENT | PTE_WRITE | PTE_LARGE;

        flush_tlb_addr(vad);
        return;
    }

    force_pte(pde, flags);

    pte_t *pte = ((pte_t *) va(pte_phys(*pde) << 12)) + pml1_i(vad);
    *pte = pad | PTE_PRESENT | PTE_WRITE;

    if (!(flags & NOT_FLUSH_TLB)) {
        flush_tlb_addr(vad);
    }
}

phys_t get_phys_address(virt_t vad) {
    phys_t pad;
    pte_t *pml4e = pml4 + pml4_i(vad);
    pte_t *pdpte = ((pte_t *) va(pte_phys(*pml4e) << 12)) + pml3_i(vad);
    pte_t *pde = ((pte_t *) va(pte_phys(*pdpte) << 12)) + pml2_i(vad);
    if (pte_large(*pde)) {
        pad = ((*pde & (~((1 << 21) - 1)))) | (vad & ((1 << 21) - 1));
        return pad;
    }

    pte_t *pte = ((pte_t *) va(pte_phys(*pde) << 12)) + pml1_i(vad);
    pad = ((*pte & (~((1 << 12) - 1)))) | (vad & ((1 << 12) - 1));;
    return pad;
}
