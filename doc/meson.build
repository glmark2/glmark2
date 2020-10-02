foreach flavor : flavors
    binary = flavor_info[flavor][0]
    config = configuration_data()
    config.set('APPNAME', binary.to_upper())
    config.set('appname', binary)
    config.set('appversion', full_version)
    f = configure_file(
        input: 'glmark2.1.in',
        output: binary + '.1',
        configuration: config
        )
    install_man(f)
endforeach
