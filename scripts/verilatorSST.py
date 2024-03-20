import os
import subprocess
import sst

verilog_component = {}


def add_component(name, element_type):
    # todo sanitize user input
    global verilog_component
    ps = subprocess.Popen(['sst-info', element_type], stdout=subprocess.PIPE)
    element_header_path = subprocess.check_output(
        ['awk', '/using file: [^ ]*/{print $NF}'], stdin=ps.stdout,).decode('utf-8')

    src_dir_path = ''
    if(os.path.exists(element_header_path)):
        src_dir_path = os.path.join(os.path.dirname(element_header_path),os.path.pardir)
    else:
        src_dir_path = os.path.join(os.path.dirname(__file__),os.path.pardir,'src')

    assert os.path.exists(src_dir_path)
    assert (not (name in verilog_component))
    verilog_component[name] = {'src_dir_path': src_dir_path, 'element_type': element_type}


def get_component(name):
    global verilog_component
    assert (name in verilog_component)
    return verilog_component[name]


def define_macro(name, key, value):
    # todo sanitize user input
    component = get_component(name)
    if (not ('macros_verilog' in component)):
        component['macros_verilog'] = ''
        component['macros_sst'] = []
    component['macros_verilog'] += f'-D{key}={value} '
    component['macros_sst'] += [[key,value]]
    pass

def define_macros(name, macros):
    for macro in macros:
        define_macro(name,macro[0],macro[1])


def compileComponent(component):
    ret_code = subprocess.call(['make', '-C', component['src_dir_path'],
                               'clean'], stderr=subprocess.STDOUT)
    assert ret_code == 0
    ret_code = subprocess.call(
        ['make', f'VERILOG_MACROS={component["macros_verilog"]}', '-C', component['src_dir_path']], stderr=subprocess.STDOUT)
    assert ret_code == 0


def finalize(name):
    component = get_component(name)
    compileComponent(component)

    sstComponent = sst.Component(name, component['element_type'])
    sstComponent = sstComponent.addParams(
        {pair[0]: pair[1] for pair in component['macros_sst']})
