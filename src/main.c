#include "ARMTinyVM.h"


int main()
{
    VM_interaction_instructions instrs = {
        .readByte = NULL,
        .writeByte = NULL,
        .softwareInterrupt = NULL
    };
    VM_instance vm = VM_new(&instrs, 0xFFFE, 0);

    return 0;
}
