import os
import subprocess
import sst

verilator_components = {}

class VerilatorSSTBuildDescription:
    def __init__(self,name,src_dir_path,element_type,install_exists):
        self.name=name
        self.src_dir_path = src_dir_path
        self.element_type = element_type
        self.macros_sst = []
        self.macros_verilog = ''
        self.install_exists = install_exists

    def __str__(self):
        return (f'src_dir_path: {self.src_dir_path}\n'
            f'element_type: {self.element_type}\n'
            f'macros: {self.macros_sst}\n'
            f'install_exists: {self.install_exists}')


def create_component(name, element_type):
    # todo sanitize user input
    global verilator_components
    ps = subprocess.Popen(['sst-info', element_type], stdout=subprocess.PIPE)
    
    element_header_path = subprocess.check_output(
        ['awk', '/using file: [^ ]*/{print $NF}'], stdin=ps.stdout).decode('utf-8')
    
    src_dir_path = ''
    element_header_path_stripped = element_header_path.strip('\n')
    install_exists = os.path.exists(element_header_path_stripped)

    if(install_exists):
        src_dir_path = os.path.join(os.path.dirname(element_header_path_stripped),os.path.pardir)
    else:
        src_dir_path = os.path.join(os.path.dirname(__file__),os.path.pardir,'src')

    assert os.path.exists(src_dir_path)
    abs_src_dir_path = os.path.abspath(src_dir_path)
    assert (not (name in verilator_components))
    verilator_components[name] = VerilatorSSTBuildDescription(
        name,
        abs_src_dir_path,
        element_type,
        install_exists)


def get_component(name):
    global verilator_components
    assert (name in verilator_components)
    return verilator_components[name]


def define_macro(name, key, value):
    # todo sanitize user input
    component = get_component(name)
    component.macros_verilog += f'-D{key}={value} '
    component.macros_sst += [[key,value]]
    pass


def define_macros(name, macros):
    for macro in macros:
        define_macro(name,macro[0],macro[1])


def write_build_data(hash_file, build_data):
    with open(hash_file,"w") as f:
        f.write(build_data)


def read_build_data(hash_file):
    data = None
    with open(hash_file,"r") as f:
        data = f.read()
    return data


def gather_src_last_modified(curr_path):
    if os.path.isdir(curr_path):
        data = ''
        for dir_entry in os.scandir(curr_path):
            data += gather_src_last_modified(dir_entry.path)
        return data
    else:
        last_modified = subprocess.check_output(['date','-u','-r',curr_path]).decode('utf-8')
        return f'{curr_path}: {last_modified}'


def smart_compile(component):
    src_last_modified_arr = gather_src_last_modified(component.src_dir_path)
    src_last_modified_str = ''.join(src_last_modified_arr)

    curr_build_data = f'{src_last_modified_str}{component}'
    old_build_data = None

    build_data_file = os.path.join(os.path.curdir,'.build_data')
    if os.path.exists(build_data_file):
        old_build_data = read_build_data(build_data_file)

    print(curr_build_data)
    print(old_build_data)
    has_changed = (curr_build_data != old_build_data)
    print(f'has_changed {has_changed}')

    if not component.install_exists or has_changed:
        compileComponent(component)
        write_build_data(build_data_file, curr_build_data)


def compileComponent(component):
    ret_code = subprocess.call(['make', '-C', component.src_dir_path,
                               'clean'], stderr=subprocess.STDOUT)
    assert ret_code == 0
    ret_code = subprocess.call(
        ['make', f'VERILOG_MACROS={component.macros_verilog}', '-C', component.src_dir_path], stderr=subprocess.STDOUT)
    assert ret_code == 0


def finalize(name):
    component = get_component(name)

    smart_compile(component)

    sstComponent = sst.Component(name, component.element_type)
    sstComponent = sstComponent.addParams(
        {pair[0]: pair[1] for pair in component.macros_sst})
