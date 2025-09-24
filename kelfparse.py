from elftools.elf.elffile import ELFFile

def extract_info(path):
    with open(path, "rb") as f:
        elf = ELFFile(f)

        entry = elf.header["e_entry"]

        # -----------------------
        # Sections: .text, .data (store full bytes)
        # -----------------------
        def sec_bytes(name):
            sec = elf.get_section_by_name(name)
            if sec is None:
                return None
            return sec.data()  # returns Python bytes object

        text_bytes = sec_bytes(".text").hex()
        data_bytes = sec_bytes(".data").hex()

        syms = {}
        for sec in elf.iter_sections():
            if not hasattr(sec, "iter_symbols"):
                continue
            for sym in sec.iter_symbols():
                if sym.name in ("kernel_trap", "argc", "argv"):
                    syms[sym.name] = sym["st_value"]

        return {
            "entry": entry,
            "textStart": elf.get_section_by_name(".text")["sh_addr"],
            "dataStart": elf.get_section_by_name(".data")["sh_addr"],
            "text": ["0x" + text_bytes[i:i+2] for i in range(0, len(text_bytes), 2)],
            "data": ["0x" + data_bytes[i:i+2] for i in range(0, len(data_bytes), 2)],
            "trapEntry": syms["kernel_trap"],
            "argc": syms["argc"],
            "argv": syms["argv"],
        }

def format_and_write(info: dict):
    with open("machine/KernelElf.h", "w") as file:
        file.writelines([
            "#ifndef __KERNELELF_H__\n",
            "#define __KERNELELF_H__\n",
            "#include <array>\n",
            "#include <cstdint>\n",
            "// Python generated file to embed the kernel elf as compile-time values\n\n",
            "namespace KernelElf {\n"
            f"\tinline constexpr uint32_t bootEntry = {info['entry']};\n",
            f"\tinline constexpr uint32_t trapEntry = {info['trapEntry']};\n",
            f"\tinline constexpr uint32_t argc = {info['argc']};\n",
            f"\tinline constexpr uint32_t argv = {info['argv']};\n",
            f"\tinline constexpr uint32_t textStart = {info['textStart']};\n",
            f"\tinline constexpr uint32_t dataStart = {info['dataStart']};\n\n",
        ])
        
        file.write(f"\talignas(4) inline constexpr std::array<uint8_t, {len(info['text'])}> kernelText""{")
        for byte in info["text"]:
            file.write(f"{byte}, ")
        file.write("};\n")
        
        file.write(f"\talignas(4) inline constexpr std::array<uint8_t, {len(info['data'])}> kernelData""{")
        for byte in info["data"]:
            file.write(f"{byte}, ")
        file.write("};\n\n}\n\n")
        
        file.write("#endif")
    

format_and_write( extract_info( "kernel/kernel.elf" ) )
    