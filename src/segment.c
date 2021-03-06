#include"segment.h"
#include"graphic.h"
#include "k_memory.h"
#include "func.h"

static struct SEGMENT_DESCRIPTOR *gdt;
static struct GATE_DISCRIPTOR *idt;

void init_gdtidt(void)
{
    gdt = (struct SEGMENT_DESCRIPTOR *)memory_allocate(
            (sizeof(struct SEGMENT_DESCRIPTOR) * NUM_GDT));
/*     struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)GDT_ADDR; */
/*     struct GATE_DISCRIPTOR *idt = (struct GATE_DISCRIPTOR *) IDT_ADDR; */
    idt = (struct GATE_DISCRIPTOR*)memory_allocate(
            (sizeof(struct GATE_DISCRIPTOR) * NUM_IDT));

    printf(TEXT_MODE_SCREEN_RIGHT, "idt: 0x%x", idt);
    printf(TEXT_MODE_SCREEN_RIGHT, "gdt: 0x%x", gdt);

    if(idt == NULL){
        return;
    }
    int i;

    //init GDT
    for(i = 0; i < NUM_GDT; i++){
        set_segmdesc(gdt + i, 0, 0, 0, 0, 0, 0, 0);
    }
    set_segmdesc(
            gdt + CODE_SEGMENT_NUM, 0xffffffff, 0x00000000, 0, SEG_TYPE_CODE_XRC,
            DESC_TYPE_SEGMENT, PRIVILEGE_LEVEL_OS, PRESENT);

    set_segmdesc(
            gdt + DATA_SEGMENT_NUM, 0xffffffff, 0x00000000, 0, SEG_TYPE_DATE_REW,
            DESC_TYPE_SEGMENT, PRIVILEGE_LEVEL_OS, PRESENT);


/*     load_gdtr(0xffff, (uintptr_t)gdt); */
    load_gdtr(sizeof(struct SEGMENT_DESCRIPTOR)*4, (uintptr_t)gdt);

    //init IDT
    for(int i = 0; i < NUM_IDT; i++){
        set_gatedesc(idt + i, (uintptr_t)io_hlt, 0, 0, 0, 0, 0);
    /*         set_gatedesc( */
/*             idt + 0x20, (uintptr_t)asm_timer_inthandler, 1*8, GATE_TYPE_32BIT_INT, 0, */
/*             PRIVILEGE_LEVEL_OS, PRESENT); */
    }


/*     for(int i = 0; i < 14; i++){ */
/*         set_gatedesc( */
/*             idt + i, (uintptr_t)asm_fault_inthandler2, 1*8, GATE_TYPE_32BIT_INT, 0, */
/*             PRIVILEGE_LEVEL_OS, PRESENT); */
/*     } */
/*     for(int i = 14; i < 20; i++){ */
/*         set_gatedesc( */
/*             idt + i, (uintptr_t)asm_fault_inthandler, 1*8, GATE_TYPE_32BIT_INT, 0, */
/*             PRIVILEGE_LEVEL_OS, PRESENT); */
/*     } */

    set_gatedesc(
            idt + 13, (uintptr_t)asm_fault_inthandler2, CODE_SEGMENT_NUM*8,
            GATE_TYPE_32BIT_INT, 0, PRIVILEGE_LEVEL_OS, PRESENT);

    set_gatedesc(
            idt + 8, (uintptr_t)asm_fault_inthandler, CODE_SEGMENT_NUM*8,
            GATE_TYPE_32BIT_INT, 0, PRIVILEGE_LEVEL_OS, PRESENT);

/*     load_idtr(IDT_LIMIT, (uintptr_t)idt); */
    load_idtr(sizeof(struct GATE_DISCRIPTOR) * NUM_IDT, (uintptr_t)idt);

    set_gatedesc(
            idt + 0x20, (uintptr_t)asm_timer_inthandler, CODE_SEGMENT_NUM*8,
            GATE_TYPE_32BIT_INT, 0, PRIVILEGE_LEVEL_OS, PRESENT);

    set_gatedesc(
            idt + 0x21, (uintptr_t)asm_inthandler21, CODE_SEGMENT_NUM*8,
            GATE_TYPE_32BIT_INT, 0, PRIVILEGE_LEVEL_OS, PRESENT);

    return;
}

void set_segmdesc(
        struct SEGMENT_DESCRIPTOR *sd, uint32_t limit,
        uint32_t base, uint8_t accessed,
        uint8_t segment_type, uint8_t descriptor_type,
        uint8_t descriptor_privilege_level, uint8_t present)
{
    if(limit > 0xfffff){
        limit /= 0x1000;
    }

    sd->limit_low = limit & 0xffff;
    sd->base_low = base & 0xffff;
    sd->base_mid = (base >> 16) & 0xff;
/*     sd->access_right = ar & 0xff; */
    sd->accessed = accessed & 0x01;
    sd->segment_type = segment_type & 0x07;
    sd->descriptor_type = descriptor_type & 0x01;
    sd->descriptor_privilege_level = descriptor_privilege_level & 0x03;
    sd->present = present & 0x01;

    sd->limit_high = ((limit >> 16) & 0x0f);

    sd->available = 0;
    sd->code_segment_for_64bit = 0;
    sd->default_operand_size = 1;
    sd->granularity = 0;


    sd->base_high = (base >> 24) & 0xff;

    return;
}

void set_gatedesc( struct GATE_DISCRIPTOR *gd, uint32_t offset,
        uint32_t selector, uint8_t gate_type,
        uint8_t storage_segment, uint8_t descriptor_privilege_level,
        uint8_t present)
{
        gd->offset_low = offset & 0xffff;
        gd->selector = selector;
        gd->gate_type = gate_type & 0xf;
        gd->storage_segment = storage_segment & 0x1;
        gd->descriptor_privilege_level = descriptor_privilege_level & 0x3;
        gd->present = present & 0x1;
        gd->offset_high = (offset >> 16) & 0xffff;

        return;
}

void init_pic(void)
{
    //Mask
    io_out8(PIC_MASTER_DATA_PORT, PIC_IMR_MASK_IRQ_ALL);
    io_out8(PIC_SLAVE_DATA_PORT, PIC_IMR_MASK_IRQ_ALL);

    //Master
    io_out8(PIC_MASTER_CMD_STATE_PORT,  PIC_MASTER_ICW1);
    io_out8(PIC_MASTER_DATA_PORT,       PIC_MASTER_ICW2);
    io_out8(PIC_MASTER_DATA_PORT,       PIC_MASTER_ICW3);
    io_out8(PIC_MASTER_DATA_PORT,       PIC_MASTER_ICW4);

    //Slave
    io_out8(PIC_SLAVE_CMD_STATE_PORT,   PIC_SLAVE_ICW1);
    io_out8(PIC_SLAVE_DATA_PORT,        PIC_SLAVE_ICW2);
    io_out8(PIC_SLAVE_DATA_PORT,        PIC_SLAVE_ICW3);
    io_out8(PIC_SLAVE_DATA_PORT,        PIC_SLAVE_ICW4);

    // setting enable
    io_out8(PIC_MASTER_DATA_PORT, 0xfc); //1111 1100
    io_out8(PIC_SLAVE_DATA_PORT, 0xff);  //1111 1111

    return;
}

void init_pit(void)
{
    set_pit_count(PIT_CLK_10MS , PIT_CONTROL_WORD_SC_COUNTER0,
            PIT_CONTROL_WORD_MODE_SQARE);

    return;
}

void set_pit_count(uint16_t count, uint8_t counter, uint8_t mode)
{
    uint8_t command;

    command = mode | PIT_CONTROL_WORD_RL_LSB_MSB | counter;

    io_out8(PIT_PORT_CONTROL_WORD, command);

    io_out8(PIT_PORT_COUNTER0, (uint8_t)(count & 0xff));
    io_out8(PIT_PORT_COUNTER0, (uint8_t)((count >> 8) & 0xff));

    return;
}


