#!/usr/bin/env python
'''
Updates icon font and header files for CopyQ repository.

First argument is path to upacked Font Awesome archive (https://fontawesome.com/).
'''
import json
import os
import sys

from shutil import copyfile


fonts_src_dest = [
    ('fa-solid-900.ttf', 'fontawesome-solid.ttf'),
    ('fa-brands-400.ttf', 'fontawesome-brands.ttf'),
]

styles = frozenset([
    'solid',
    'brands',
])


def has_style(icon, styles):
    icon_set = frozenset(icon['styles'])
    return not icon_set.isdisjoint(styles)


def read_icons(icons_json, styles):
    with open(icons_json, 'r') as icons_file:
        icons_content = icons_file.read()
        icons = json.loads(icons_content)
        icons = {name: icon for name, icon in icons.items() if has_style(icon, styles)}
        return icons


def write_header_file_preamble(header_file):
    script = os.path.realpath(__file__)
    script_name = os.path.basename(script)
    comment = (
            f'// This file is generated with "{script_name}"'
            + ' from FontAwesome\'s metadata.\n\n')
    header_file.write(comment)


def write_add_icons_header_file(header_add_icons, icons):
    with open(header_add_icons, 'w') as header_file:
        write_header_file_preamble(header_file)
        header_file.write('// List of method calls for IconSelectDialog.\n')

        for name, icon in icons.items():
            code = icon['unicode']
            search_terms = [icon['label'].lower()] + icon['search']['terms']
            search_terms_list = (
                    'QStringList()'
                    + ''.join([' << ' + json.dumps(term) for term in search_terms]))
            header_file.write(f'addIcon(0x{code}, {search_terms_list});' + '\n')


def write_icons_header_file(header_icons, icons):
    with open(header_icons, 'w') as header_file:
        write_header_file_preamble(header_file)
        header_file.write('#ifndef ICONS_H\n')
        header_file.write('#define ICONS_H\n')
        header_file.write('\n')
        header_file.write('enum IconId {\n')

        for name, icon in icons.items():
            label = name.title().replace('-', '')
            code = icon['unicode']
            header_file.write(f'    Icon{label} = 0x{code},' + '\n')

        header_file.write('};\n')
        header_file.write('\n')
        header_file.write('#endif // ICONS_H\n')


def copy_fonts(font_awesome_src, target_font_dir):
    font_dir = os.path.join(font_awesome_src, 'web-fonts-with-css', 'webfonts')
    for src_name, dest_name in fonts_src_dest:
        src_path = os.path.join(font_dir, src_name)
        dest_path = os.path.join(target_font_dir, dest_name)
        print(f'Copying: {src_path} -> {dest_path}')
        copyfile(src_path, dest_path)


def main():
    font_awesome_src = sys.argv[1]

    script = os.path.realpath(__file__)
    utils_dir = os.path.dirname(script)
    src_dir = os.path.join(utils_dir, '..', 'src')

    header_add_icons = os.path.join(src_dir, 'gui', 'add_icons.h')
    header_icons = os.path.join(src_dir, 'gui', 'icons.h')

    target_font_dir = os.path.join(src_dir, 'images')
    copy_fonts(font_awesome_src, target_font_dir)

    icons_json = os.path.join(
            font_awesome_src, 'advanced-options', 'metadata', 'icons.json')
    icons = read_icons(icons_json, styles)

    write_add_icons_header_file(header_add_icons, icons)
    print(f'Header file "{header_add_icons}" updated.')

    write_icons_header_file(header_icons, icons)
    print(f'Header file "{header_icons}" updated.')


if __name__ == '__main__':
    main()
