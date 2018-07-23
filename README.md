# realmz_dasm

This project contains tools I wrote for reverse-engineering classic Mac OS games. There are currently three tools:

- realmz_dasm, a disassembler for Realmz scenarios
- resource_dump, a disassembler for resource forks
- render_infotron_levels, a map generator for Infotron levels

## realmz_dasm

Realmz is a fantasy role-playing game for Windows and classic Mac OS. realmz_dasm is a disassembler for Realmz scenarios; it produces annotated maps of all land and dungeon levels, as well as descriptions of all events and encounters that may occur in the scenario.

To use realmz_dasm:
- Install Netpbm (http://netpbm.sourceforge.net/).
- Build and install phosg (https://github.com/fuzziqersoftware/phosg).
- Build realmz_dasm (run `make`) and put it in the same directory as Realmz.
- Run disassemble_all.sh in that directory.

This will produce a directory named "Disassembly" containing some very large image files (maps of all the land and dungeon levels), the scenario script, and the resources contained in each scenario (icons, sounds, text).

## resource_dump

resource_dump is a disassembler for classic Mac OS resource forks. realmz_dasm uses it to parse tile images to produce map images, but resource_dump can also be used by itself.

Building resource_dump is the same as above. resource_dump can extract raw resources from any file's resource fork and decode some resources into modern formats:
- Converts cicn, ICON, SICN, ics#/4/8, icl4/8, CURS, crsr, PAT, PAT#, ppat, and PICT resources to bmp files. May fail on icons with nonstandard sizes or formats, or PICTs containing unusual opcodes.
- Converts snd resources to wav files.
- Converts TEXT resources to txt files with Unix line endings.

resource_dump attempts to transparently decompress resources that are stored in compressed formats. Current support for decompression is incomplete; it depends on an embedded MC68K emulator that doesn't (yet) implement the entire CPU. If you use resource_dump and it fails on a compressed resource, send me the file and I'll add support for it.

Run resource_dump without any arguments for usage information.

## render_infotron_levels

Infotron is a puzzle game very much like Supaplex (and Move Blocks and Eat Stuff). render_infotron_levels decodes the levels from the game's resource fork and draws maps of them. Just put render_infotron_levels in the "Info Datafiles" folder and run it from there.