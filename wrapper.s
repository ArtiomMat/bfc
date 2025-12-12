.global _start      /* Export the entry point symbol */

.section .text      /* Place this in the executable code section */

_start:
    .incbin "bfcbin" /* Include your raw binary bytes right here */
