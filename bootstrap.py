import sys
import os
import errno
import platform
import argparse

from bootstraper import bootstraper
# python 3.3.2+
def isWin():
    return 'Windows' in platform.system()

# python 3.3.2+ 64 bits
def isLinux():
    return 'Linux' in platform.system()

# python 3.4.1 64 bits
def isDarwin():
    return 'Darwin' in platform.system()


def main():
    parser = argparse.ArgumentParser(description="Generate the project files for the current platform .")
    if(isWin()):
        parser.add_argument('target', choices=['win','win64'],help = "On the current platform, the types that can be built.")
        parser.add_argument('--cpu_compile_cores',default = 12,type = int,help="Specifies the number of compiled cores.")
    elif(isLinux()):
        parser.add_argument('target', choices=['linux','android','hisiv500'],help="On the current platform, the types that can be built.")
    elif(isDarwin()):
        parser.add_argument('target', choices=['osx','ios','android','win','win64','linux',"hisiv500"],help="On the current platform, the types that can be built.")
    else:
        parser.add_argument('target', choices=['osx','win','win64','android','ios','linux','unix'],help = "On the current platform, the types that can be built.")
    
    parser.add_argument('--folder',default='build', help='The build folder, relative path of project directory.Default is "./build".')
    parser.add_argument('--build',action='store_true', help='Build after project files genarated.')
    parser.add_argument('--debug',action='store_true', help='Enable Debug build.')

    args = parser.parse_args()

    if '--help' in sys.argv or '-h' in sys.argv:
        parser.print_help()
        sys.exit()

    try:
        cores = 1;
        if isWin():
            cores = args.cpu_compile_cores;
        bootrun = bootstraper(
            target=args.target,
            build_folder = args.folder,
            build = args.build,
            debug = args.debug,
            compiler_cores=cores
        )
        bootrun.run()
    except OSError as e:
        print(e)
        sys.exit()

if __name__ == "__main__":
    sys.exit(main())
