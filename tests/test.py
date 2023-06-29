import json
import subprocess
import os.path


def main():
    # Load the tests file
    # This will be a dictionary mapping strings to integers
    with open("tests.json", "rt") as test_file:
        tests = json.load(test_file)

    # Each key is the relative address of an assembly file
    # Each value is the intended exit value when that program is executed
    for asm_filename_full, expected_result in tests.items():
        # Work out what we'll call the resulting object file and executable file
        directory, asm_filename = os.path.split(asm_filename_full)
        gen_filename = asm_filename.split(".")[0]
        obj_filename_full = os.path.join(directory, gen_filename + ".o")
        elf_filename_full = os.path.join(directory, gen_filename + ".elf")

        # We need to assemble the .s file and link it to the start.o file in this directory
        subprocess.run(["arm-none-eabi-as", "-mthumb", "-o", obj_filename_full, asm_filename_full])
        subprocess.run(["arm-none-eabi-ld", obj_filename_full, "start.o", "-o", elf_filename_full])

        # Then we execute that using qemu and see the result
        return_code = subprocess.run(["qemu-arm", elf_filename_full]).returncode

        # Does the result match what we wanted?
        if return_code == expected_result:
            print(f"TEST SUCCESS: {asm_filename_full} -> {return_code}")
        else:
            print(f"TEST FAILED: {asm_filename_full} -> {return_code}, supposed to be {expected_result}")


if __name__ == "__main__":
    main()
