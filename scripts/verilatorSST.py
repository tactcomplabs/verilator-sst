import os
import subprocess
import sst

verilog_component = {}


def SSTVerilogComponent(name, element_type):
    # todo sanitize user_input
    add_component(name, element_type)


def add_component(name, element_type):
    global verilog_component
    ps = subprocess.Popen(['sst-info', element_type], stdout=subprocess.PIPE)
    element_header_path = subprocess.check_output(
        ['awk', '/using file: [^ ]*/{print $NF}'], stdin=ps.stdout).decode('utf-8')
    lib_dir_path = os.path.dirname(element_header_path)
    print(lib_dir_path)
    assert os.path.exists(lib_dir_path)
    assert (not (name in verilog_component))
    verilog_component[name] = {'lib_dir_path': lib_dir_path,
                               'element_type': element_type}


def get_component(name):
    global verilog_component
    assert (name in verilog_component)
    return verilog_component[name]


def define_macro(name, macros):
    # todo sanitize user input
    component = get_component(name)
    if (not ('macros' in component)):
        component['macros'] = []
    component['macros'] = macros + component['macros']
    pass


def compileComponent(component):
    macros_str = '+define'

    for pair in component['macros']:
        macros_str += f'+{pair[0]}={pair[1]}'

    ret_code = subprocess.call(['make', '-C', component['lib_dir_path'],
                               'clean'], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
    assert ret_code == 0
    ret_code = subprocess.call(
        ['make', f'VERILOG_MACROS={macros_str}', '-C', component['lib_dir_path'], 'install'])
    assert ret_code == 0


def finalize(name):
    component = get_component(name)
    if (component['macros'] != None):
        compileComponent(component)

    sstComponent = sst.Component(name, component['element_type'])
    sstComponent = sstComponent.addParams(
        {pair[0]: pair[1] for pair in component['macros']})
    return sstComponent
