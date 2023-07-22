import json
import subprocess
import os.path
import sys
from glob import glob
import re
from typing import Optional


def main():
    # Compile start.s and lib.s fresh, so any changes are used
    subprocess.run(["arm-none-eabi-as", "-o", "start_arm.o", "start.s"])
    subprocess.run(["arm-none-eabi-as", "-mthumb", "-o", "start_thumb.o", "start.s"])
    subprocess.run(["arm-none-eabi-as", "-mthumb", "-o", "lib.o", "lib.s"])

    # Find a list of tests, mapping filename to expected return code
    # The first line of the .s or .c file should contain a comment with the expected return value
    tests = {}
    for filename in glob("./instructions/**/*.[cs]", recursive=True):
        expected_return = get_test_expected_result(filename)
        if expected_return:
            tests[filename] = expected_return
    for filename in glob("./lib/**/*.[cs]", recursive=True):
        expected_return = get_test_expected_result(filename)
        if expected_return:
            tests[filename] = expected_return
    print(tests)

    # Each key is the relative address of an assembly or C file
    # Each value is the intended exit value when that program is executed
    for test_filename_full, expected_result in tests.items():
        # Work out what we'll call the resulting object file and executable file
        directory, asm_filename = os.path.split(test_filename_full)
        gen_filename = asm_filename.split(".")[0]
        obj_filename_full = os.path.join(directory, gen_filename + ".o")
        elf_filename_full = os.path.join(directory, gen_filename + ".elf")

        # We need to assemble or compile the test file and link it to the start.o file in this directory
        if test_filename_full.endswith(".s"):
            subprocess.run(["arm-none-eabi-as", "-mthumb", "-mcpu=arm7tdmi", "-o", obj_filename_full, test_filename_full])
        elif test_filename_full.endswith(".c"):
            subprocess.run(["arm-none-eabi-gcc", "-mthumb", "-Os", "-mcpu=arm7tdmi", "-nostdlib", "-c", "-o", obj_filename_full, test_filename_full])
        else:
            raise Exception(f"Unknown test file type: {test_filename_full}")

        # Link for QEMU (with ARM-to-Thumb start code) and for the VM (starting with the main function)
        subprocess.run(["arm-none-eabi-ld", "start_arm.o", "lib.o", obj_filename_full, "-o", elf_filename_full.replace(".elf", ".qemu.elf")])
        subprocess.run(["arm-none-eabi-ld", "start_thumb.o", "lib.o", obj_filename_full, "-o", elf_filename_full.replace(".elf", ".sim.elf")])

        # Then we execute that using qemu and see the result
        return_code_direct_simulation = subprocess.run(["qemu-arm", elf_filename_full.replace(".elf", ".qemu.elf")]).returncode

        # Run it also with the Windows-compiled ARMTinyVM
        return_code_vm_win = subprocess.run(["../cmake-build-debug-mingw/ARMTinyVM.exe", elf_filename_full.replace(".elf", ".sim.elf")]).returncode
        return_code_vm_wsl = subprocess.run(["../cmake-build-debug-for-wsl/ARMTinyVM", elf_filename_full.replace(".elf", ".sim.elf")]).returncode

        # Does the result match what we wanted?
        if return_code_direct_simulation == expected_result:
            print(f"TEST SUCCESS (QEMU): {test_filename_full} -> {return_code_direct_simulation}", file=sys.stderr)
        else:
            print(f"TEST FAILED (QEMU): {test_filename_full} -> {return_code_direct_simulation}, supposed to be {expected_result}", file=sys.stderr)

        if return_code_vm_win == expected_result:
            print(f"TEST SUCCESS (WIN): {test_filename_full} -> {return_code_vm_win}", file=sys.stderr)
        else:
            print(f"TEST FAILED (WIN): {test_filename_full} -> {return_code_vm_win}, supposed to be {expected_result}", file=sys.stderr)

        if return_code_vm_wsl == expected_result:
            print(f"TEST SUCCESS (WSL): {test_filename_full} -> {return_code_vm_wsl}", file=sys.stderr)
        else:
            print(f"TEST FAILED (WSL): {test_filename_full} -> {return_code_vm_wsl}, supposed to be {expected_result}", file=sys.stderr)


def get_test_expected_result(filename: str) -> Optional[int]:
    """
    Takes the name of a .s or .c test file, and returns the expected result. This requires the first line of the file
    to be a comment containing an integer which is the return value.
    :param filename:
    :return:
    """
    if os.stat(filename).st_size == 0:
        print(f"Empty test file {filename}", file=sys.stderr)
        return None
    with open(filename, "rt") as file:
        first_line = file.readline()
        number = re.search(r"\d+", first_line)
        if number is None:
            print(f"No expected result found in {filename}", file=sys.stderr)
            return None
        return int(number.group(0))


if __name__ == "__main__":
    main()
