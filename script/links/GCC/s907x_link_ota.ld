ENTRY(system_entry)

INCLUDE "s907x_bin_api.txt"
GROUP (
  libgcc.a
  libc.a
  libg.a
  libm.a
  libnosys.a
)

MEMORY
{
    ROM (rx) :               ORIGIN = 0x00000000, LENGTH = 0x50000    /* ROM: 320k */
    OTA_RAM (rwx)  :         ORIGIN = 0x10015900, LENGTH = 0x31700    /* OTA_RAM RAM: 197k */
    WLAN (rx)  :             ORIGIN = 0x18010000, LENGTH = 0x50000    /* wlan: 320k */
    OTA1 (rx)  :             ORIGIN = 0x18060020, LENGTH = 0xD0000-0x20    /* OTA1: 832k */
    OTA2 (rx)  :             ORIGIN = 0x18130020, LENGTH = 0xD0000-0x20    /* OTA2: 832k */
}

SECTIONS
{
    .system.entry :
    {
        KEEP(system_start.o(.system.entry.data))
    } > OTA_RAM

    .ota1_ram.data : 
    {
        __data_start__ = .;
        *(.data*)
        __data_end__ = .;
        . = ALIGN(16);    
    } > OTA_RAM
  
    .ota1_ram.bss :
    {
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        __bss_end__ = .;
    } > OTA_RAM

    . = ALIGN(8);

    PROVIDE(heap_start      = .);
    PROVIDE(end             = 0x10046FFF - 0x2000);
	PROVIDE(heap_end        = end);
    
    .ota1_image.text : 
    {
        __ota1_text_start__ = .;

        *(.text*)    
        *(.rodata*)
        *(.debug_trace*)
        
        __ota1_text_end__ = .;
    
        . = ALIGN (16);
    } > OTA1
}


