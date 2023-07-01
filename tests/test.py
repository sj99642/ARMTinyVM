import json
import subprocess
import os.path


def main():
    # Compile start.s and lib.s fresh, so any changes are used
    subprocess.run(["arm-none-eabi-as", "-o", "start.o", "start.s"])
    subprocess.run(["arm-none-eabi-as", "-mthumb", "-o", "lib.o", "lib.s"])

    # Load the tests file
    # This will be a dictionary mapping strings to integers
    with open("tests.json", "rt") as test_file:
        tests = json.load(test_file)

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
        subprocess.run(["arm-none-eabi-ld", "-e", "_start", "start.o", "lib.o", obj_filename_full, "-o", elf_filename_full])

        # Then we execute that using qemu and see the result
        return_code = subprocess.run(["qemu-arm", elf_filename_full]).returncode

        # Does the result match what we wanted?
        if return_code == expected_result:
            print(f"TEST SUCCESS: {test_filename_full} -> {return_code}")
        else:
            print(f"TEST FAILED: {test_filename_full} -> {return_code}, supposed to be {expected_result}")


if __name__ == "__main__":
    main()
