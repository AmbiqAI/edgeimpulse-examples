#!/usr/bin/env python3

import argparse

#******************************************************************************
#
# Main function.
#
#******************************************************************************
def main():

    pinlist = []
    c_template = ''
    h_template = ''

    formatlist = dict()

    # Read the arguments from the command line.
    args = parse_arguments()

    # Open the CSV file containing the pin information.
    csvdict = csv_to_dicts(args.csv)

    # Cut off the first row, and transform the other rows into pin objects.
    pinlist = map(pin, csvdict)

    # Filter out any pins that aren't named
    pinlist = list(pin for pin in pinlist if pin.name)

    # Split the list into groups
    pingroups = split_pin_groups(pinlist)

    # Write a set of pin definitions for each group, and insert them into the
    # header file template.
    groups = '\n'.join(write_group_pindefs(g) for g in pingroups)
    hfile_text = header_template.format(groups = groups)

    with open('am_bsp_gpio.h', 'w', newline = '') as hfile:
        print(hfile_text, file=hfile)

#******************************************************************************
#
# Generate commmand line options, and parse arguments.
#
#******************************************************************************
def parse_arguments():

    parser = argparse.ArgumentParser()

    parser.add_argument('csv', help='Name of the pin definition csv file.')

    return parser.parse_args()

#******************************************************************************
#
# Takes a list of pin objects, sorts it by group, and returns a list of lists
# of pin objects, where each list only contains pins from a single group.
#
#******************************************************************************
def split_pin_groups(pins):

    # Sort the pin list by group
    sortkey = lambda x: x.groupname
    sorted_pins = sorted(pins, key = sortkey)

    # Start with the group of the first pin.
    groups = [[]]
    current_groupname = sorted_pins[0].groupname

    # Loop through the pins.
    for p in sorted_pins:
        # If we're still looking at pins from the same group, just add them to
        # the list.
        if p.groupname == current_groupname:
            groups[-1].append(p)

        # If this pin is from a new group, start a new sub-list.
        else:
            groups.append([])
            groups[-1].append(p)

            # Change the current groupname
            current_groupname = p.groupname

    # Sort the old pin list by group, and add it to "groups"
    sortkey = lambda x: x.name
    sortgroup = lambda x: sorted(x, key = sortkey)

    sorted_groups = [sortgroup(g) for g in groups]

    return sorted_groups

#******************************************************************************
#
# Takes a list of pin objects, sorts it by group, and returns a list of lists
# of pin objects, where each list only contains pins from a single group.
#
#******************************************************************************
def write_group_pindefs(group):

    pindefs = ''

    # The output will be a little different for un-grouped pins.
    if group[0].groupname == '':
        # Loop through the pins in the group.
        for pin in group:

            # Write a macro for the pin number.
            macroname = 'AM_BSP_GPIO_{}'.format(pin.name)
            pindefs += macrodefine.format(name = macroname, value = pin.num) + '\n'

        return groupblock.format(groupname = 'Miscellaneous',
                                 pins = pindefs)
    else:
        # Loop through the pins in the group.
        for pin in group:
            # Write a macro for the pin number.
            macroname = 'AM_BSP_GPIO_{}_{}'.format(pin.groupname, pin.name)
            pindefs += macrodefine.format(name = macroname, value = pin.num) + '\n'

            # Write another macro for the configuration value.
            macroname = 'AM_BSP_GPIO_CFG_{}_{}'.format(pin.groupname, pin.name)
            pindefs += macrodefine.format(name = macroname, value = pin.function) + '\n'

            # If provided, write a macro for the SPI channel number
            if pin.SpiChannel:
                macroname = 'AM_BSP_{}_{}_CHNL'.format(pin.groupname, pin.name)
                pindefs += macrodefine.format(name = macroname, value = pin.SpiChannel) + '\n'

        return groupblock.format(groupname = group[0].groupname,
                                 pins = pindefs)


#******************************************************************************
#
# Print a gpio config line for a single pin.
#
#******************************************************************************
def write_pinconfig(p):

    # Set the basic format of the pin configuration string.
    callstring = 'am_hal_gpio_pin_config({}, {});'.format(p.num, p.function)

    formatlist = dict()
    formatlist['call'] = callstring
    formatlist['comment'] = p.comment

    S = '    {call:<50} // {comment}'.format(**formatlist)

    S = S.rstrip()

    return S

#******************************************************************************
#
# Print a set of macros given a list of pins
#
#******************************************************************************
def write_macros(P):

    # Get rid of any pins without comments.
    pincomment = lambda x: x.comment
    commented_pins = list(filter(pincomment, P))

    # Convert each commented pin into a list of macro strings
    macrolists = map(pin_to_macrolist, commented_pins)

    # Merge the macro strings into a single list
    macrolist = []
    for m in macrolists:
        macrolist += m

    macrolist.sort()

    return '\n'.join(macrolist)

#******************************************************************************
#
# csv_to_dicts takes a filename for a csv file and returns a list of
# dictionaries.
#
# There is one dictionary created for each row, and one key+value pair for each
# cell. The key is the column header, and the value is the string
# representation of the cell contents. This means you can access cell contents
# like this: dict_list[row_number]['column_name']
#
#******************************************************************************
def csv_to_dicts(filename):
    dict_list = []

    with open(filename) as f:
        lines = list(line.strip().split(',') for line in f)

        #
        # Strip any comment lines from the csv (i.e. if char in col 0 is '#')
        #
        newlines = []
        for ix in range(0,len(lines)):
            if lines[ix][0][0] != '#':
                newlines.append(lines[ix])
        lines = newlines

        dict_list = list(dict(zip(lines[0], L)) for L in lines[1:])

    return dict_list

#******************************************************************************
#
# Support class for reading pin configuration files.
#
#******************************************************************************
class pin:
    def __init__(self, row):
        # Initialize all class variables.
        self.num = 0
        self.function = 'AM_HAL_PIN_DISABLE'
        self.packages = 'ALL'
        self.SpiChannel = 0

        # Pin number comes from the PIN column
        self.num = row['PIN']

        # Selected function is determined by the SEL column
        select = row['SEL']

        # NOTE - If no select function is specified, default to input.
        if select == 'I' or select == 'A' or select == '':
            self.function = 'AM_HAL_PIN_INPUT'
        elif select == 'O':
            self.function = 'AM_HAL_PIN_OUTPUT'
        elif select == 'OD':
            self.function = 'AM_HAL_PIN_OPENDRAIN'
        elif select == 'P':
            self.function = 'AM_HAL_PIN_' + self.num + '_' + row['POWER']
        else:
            # If the function was listed by number, find the name in the column
            # with the corresponding function number. The 'split' removes the
            # pin function qualifiers from the name.
            funcname = row[select].split()[0]

            # Compose the correct function macro from the pin number and
            # function name.
            self.function = 'AM_HAL_PIN_{}_{}'.format(self.num, funcname)

        # Put together the additional options field.
        optlist = row['OPTIONS'].split()

        # If there were any additional options, join them with the function
        # string.
        optstrings = []
        if optlist:
            for x in optlist:
                if x[:7] == 'outcfg-':
                    # Construct the OUTCFG enum as it exists in am_reg_gpio.h
                    pinnum = int(self.num)
                    gpiocfgletter = str(chr(ord('A') + int((pinnum / 8))))

                    #
                    # The GPIO CONFIG value fields are currently arranged as follows:
                    # [ 7: 0] PADREG configuration.
                    # [11: 8] GPIOCFG
                    # [15:12] Unused.
                    # [23:16] ALTPADREG configuration.
                    #
                    # This GPIOCFG value needs to be located in bit position 8,
                    # which may require either a right or left shift of the enum.
                    #
                    shiftval = ((int(self.num) % 8) * 4) - 8
                    shiftstr = " >> "
                    if shiftval < 0:
                        shiftval = -shiftval
                        shiftstr = " << "
                    optstrings.append("(AM_REG_GPIO_CFG" + gpiocfgletter + "_GPIO" + str(self.num) + "OUTCFG_" + x[7:] + shiftstr + str(shiftval) + ")")

                    #
                    # Now if the outcfg value is something like "M0nCE3", we'll
                    # create another define designating the CS Channel number.
                    #
                    if x[7:8] == 'M' and x[9:12]=='nCE':
                        self.SpiChannel = x[12:13]
                    elif x[7:14] == 'MSPInCE':
                        self.SpiChannel = x[14:15]
                else:
                    optstrings.append('AM_HAL_GPIO_{}'.format(x))

            self.function = '({} | {})'.format(self.function, ' | '.join(optstrings))

        # Read the rest of the columns to populate the other information.
        self.packages = row['PACKAGES']
        self.groupname = row['GROUP']
        self.name = row['NAME']

#******************************************************************************
#
# Template strings.
#
#******************************************************************************

header_template = '''
//*****************************************************************************
//
//! @file am_bsp_gpio.h
//!
//! @brief Functions to aid with configuring the GPIOs.
//
//*****************************************************************************

//*****************************************************************************
//
// ${{copyright}}
//
// This is part of revision ${{version}} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_BSP_GPIO_H
#define AM_BSP_GPIO_H

#ifdef __cplusplus
extern "C"
{{
#endif

{groups}
//*****************************************************************************
//
// Convenience macros for enabling and disabling pins by function.
//
//*****************************************************************************
#define am_bsp_pin_enable(name)                                               \\
    am_hal_gpio_pin_config(AM_BSP_GPIO_ ## name, AM_BSP_GPIO_CFG_ ## name);

#define am_bsp_pin_disable(name)                                              \\
    am_hal_gpio_pin_config(AM_BSP_GPIO_ ## name, AM_HAL_PIN_DISABLE);

#ifdef __cplusplus
}}
#endif

#endif // AM_BSP_GPIO_H
'''.strip()

groupblock = '''
//*****************************************************************************
//
// {groupname} pins.
//
//*****************************************************************************
{pins}
'''.strip()

macrodefine = '''
#define {name:31} {value}
'''.strip()

footerblock = '''
'''.strip()

if __name__ == '__main__':
    main()
