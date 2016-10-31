# SandBox
THis is a SandBox application capable of blocking illegal file and resource accesses (defined by configuration file). System calls were captured and their parameters were evaluated to ensure the access was within the boundaries dictated by the configuration file. Offending system calls would return error access codes (mainly EACCESS) to the program.
