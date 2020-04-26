project(
    'Práctica 4 de PAV - reconocimiento y verificación del locutor', 'cpp',
    default_options: 'cpp_std=c++11',
    version: 'v2.0'
)

inc = include_directories(['src/include', 'src/pav/docopt_cpp'])

# Librería PAV
subdir('src/pav')

# Programas del proyecto
subdir('src')

# Scripts de la práctica
subdir('scripts')

# Documentación del proyecto
subdir('src/doxyfile')

# Red neuronal de Santi Pascual
subdir('pav_spkid_pytorch')
