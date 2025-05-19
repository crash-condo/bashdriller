# BashDriller v2.0

**BashDriller** is a command-line training utility for practicing Linux commands via repetition based drills. Version 2.0 introduces modular `.drill` files for defining drills by category.

## Features

- Interactive command typing with validation
- Supports `showit`, `expit`, `next`, `quit` during drills
- Modular `.drill` file loading from `/etc/bashdriller/`
- Drill menu auto-generates from available `.drill` files
- Errors in drill files are logged to `/var/log/messages`

## .drill Format

Each `.drill` file must live in `/etc/bashdriller/` and be named using the pattern `name.drill` (e.g., `ls.drill`). Files must use the following format:

```
(1)
desc="List all files including hidden ones"
exp_comm="ls -a"
expl="Lists all files including those starting with a dot"
ref=""
tags="ls hidden dotfiles"
(2)
desc="List files in long format"
exp_comm="ls -l"
expl="Displays file details including permissions, owner, size, and timestamp"
ref=""
tags="ls long format permissions"
```

Fields must appear in this exact order. No blank lines are required between blocks. Entries are numbered to define order.

## Drill Menu Behavior

On launch, BashDriller:

1. Scans `/etc/bashdriller/` for all `*.drill` files  
2. Sorts them alphabetically  
3. Displays available drill categories based on filenames  

## Runtime Commands

While in a drill:

- `showit` — display expected command  
- `expit` — display full explanation  
- `next` — skip to next drill in current set  
- `quit` — return to main menu without saving progress  

## Error Handling

Malformed `.drill` files or blocks are skipped. Errors are logged via `logger` to `/var/log/messages` in the form:

```
bashdriller: malformed entry in /etc/bashdriller/<file> (entry <n>)
```

## Install Instructions

1. Build the binary:
   ```bash
   gcc -o bashdriller src/main.c src/cJSON.c
   ```

2. Move the binary:
   ```bash
   sudo mv bashdriller /usr/local/bin/
   ```

3. Create the drill directory:
   ```bash
   sudo mkdir -p /etc/bashdriller/
   ```

4. Place your `.drill` files in `/etc/bashdriller/`:
   ```bash
   sudo cp mydrills/*.drill /etc/bashdriller/
   ```

5. Run BashDriller:
   ```bash
   bashdriller
   ```

## License

GPL-3.0  
(c) Crash Condo 2025

