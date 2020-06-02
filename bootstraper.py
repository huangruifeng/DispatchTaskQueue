import sys
import os
import errno
import re
import subprocess

class bootstraper:
    target = None
    build_folder = None
    build = False
    debug = False
    compiler_cores = -1

    src_dir = os.getcwd()
    if('ANDROID_NDK_HOME' in os.environ):
        android_ndk = os.environ["ANDROID_NDK_HOME"]
    else:
        android_ndk = ""

    target_configs = {
        'osx':{'generator':'Xcode','options':['CMAKE_OSX_SYSROOT=macosx','CMAKE_OSX_ARCHITECTURES=x86_64','MACOS=ON']},
        
        'win':{'generator':'Visual Studio 15 2017', 'options':['WIN32=ON']},
        
        'win64':{'generator':'Visual Studio 15 2017 Win64', 'options':['WIN64=ON']},
        
        #'android':{'generator':'Unix Makefiles', 'options':['ANDROID_TOOLCHAIN=gcc','ANDROID_STL=gnustl_static','ANDROID_ABI=armeabi-v7a',
        #'ANDROID_NATIVE_API_LEVEL=21','CMAKE_TOOLCHAIN_FILE=%s'%(os.path.join(android_ndk, "build/cmake/android.toolchain.cmake"))]},
        
        #'android64':{'generator':'Unix Makefiles','options':['ANDROID_TOOLCHAIN=gcc', 'ANDROID_STL=gnustl_static',
        #'ANDROID_ABI=arm64-v8a', 'ANDROID_NATIVE_API_LEVEL=21', 'CMAKE_TOOLCHAIN_FILE=%s'%(os.path.join(android_ndk, "build/cmake/android.toolchain.cmake"))]},
        
        'linux':{'generator':'Unix Makefiles', 'options':['CMAKE_C_FLAGS=-fPIC', 'CMAKE_CXX_FLAGS=-fPIC', 'CMAKE_SHARED_LINKER_FLAGS="-Wl,-Bsymbolic"', 'LINUX=ON']},

        'unix':{'generator':'Unix Makefiles', 'options':['CMAKE_C_FLAGS=-fPIC', 'CMAKE_CXX_FLAGS=-fPIC', 'CMAKE_SHARED_LINKER_FLAGS="-Wl,-Bsymbolic"', 'UNIX=ON']},
        
    }

    compile_config = {
    'osx' : { 
      'Debug' : ['-configuration Debug', '-target "ALL_BUILD"', 'DEBUG_INFORMATION_FORMAT="dwarf-with-dsym"', '-UseModernBuildSystem=NO'], 
      'Release' : ['-configuration Release', '-target "ALL_BUILD"', 'DEBUG_INFORMATION_FORMAT="dwarf-with-dsym"', '-UseModernBuildSystem=NO']
    }
    }

    linux_P = ['android','android64','linux','unix']

    macos_P = ['osx','ios']

    windows_P = ['win','win64']

    def mkdir_p(self):
        try:
            os.makedirs(self.build_folder)
        except OSError as exc:
            if exc.errno == errno.EEXIST and os.path.isdir(self.build_folder):
                pass
            else:
                raise('Can not create directory.')

    def remove_cache_p(self):
        try:
            makecachefile = self.build_folder + "/CMakeCache.txt"
            os.remove(makecachefile)
            print("remove cmake cache flie:" , makecachefile)
        except OSError as exc:
            pass

    def __init__(self,target, build_folder, build, debug, compiler_cores):
        self.target = target
        self.build_folder = os.path.join(build_folder, target)
        self.build = build
        self.debug = debug
        self.compiler_cores = int(compiler_cores)
        

    def update_source(self):
        #update submodule
        os.system("git submodule update --init --recursive")

    def update_version(self):
        version_file = open(self.src_dir + '/version', 'r+')
        if version_file:
            version_data = version_file.readline()
            version_pattern = r'[0-9]{1,2}\.[0-9]{1,2}\.[0-9]{1,3}'
            match_obj = re.match(version_pattern,version_data)
            if match_obj:
                proc=subprocess.Popen("git log -1 --pretty=%H", shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, universal_newlines=True)
                commitid = proc.stdout.readline().strip()[0:8]
                proc.stdout.close()
                proc.wait()
                new_version = match_obj.group() + "." + commitid
                version_file.seek(0,0)
                version_file.truncate()
                version_file.write(new_version)
                version_file.close()
                return 0
        return -1

    def gen_project_files(self):
        cmd = 'cmake %s -G "%s"'%(self.src_dir,self.target_configs[self.target]['generator'])
        optprefix = '-D'

        options = self.target_configs[self.target]['options']

        if self.debug:
            options.append('CMAKE_BUILD_TYPE=Debug')
        else:
            options.append('CMAKE_BUILD_TYPE=Release')

        for opt in options:
            cmd += ' %s%s'%(optprefix,opt)

        print('Use this cmake command to generate project files: %s'%cmd)
        return subprocess.call(cmd,shell=True)

    def build_project(self):
        build_type = 'Release'
        if self.debug:
            build_type = 'Debug'
        
        if self.target in self.macos_P:
            cmd = 'xcodebuild clean build -project ""' # project name
            compile_configs = self.compile_config[self.target][build_type]
            
            for compile_opt in compile_configs:
                cmd = '%s %s'%(cmd, compile_opt)
        else:
            cmd = 'cmake --build . --config ' + build_type

        if self.target in self.linux_P:
            thread_sum = os.popen("grep processor /proc/cpuinfo | awk '{field=$NF};END{print field}'").read()
            cmd += ' -- -j' + thread_sum
        if self.target in self.windows_P and self.compiler_cores > 0:
            cmd += ' -- /maxcpucount:' + str(self.compiler_cores)
        print(cmd)
        return subprocess.call(cmd, shell=True)
        
    def run(self):
        self.update_source()
        self.update_version()
        self.mkdir_p()
        self.remove_cache_p()

        os.chdir(self.build_folder)

        status = self.gen_project_files()
        if(0!=status):
            raise('Generate project files failed.')

        if self.build:
            status = self.build_project()
            if 0 != status:
                raise('Build project failed.')
        print("run compltet !")

        




        