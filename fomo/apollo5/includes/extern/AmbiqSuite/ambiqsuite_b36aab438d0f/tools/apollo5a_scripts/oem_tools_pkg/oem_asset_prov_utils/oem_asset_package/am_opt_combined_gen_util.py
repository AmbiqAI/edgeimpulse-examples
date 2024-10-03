import sys
import os 

# Definitions for paths
if sys.platform != "win32" :
    path_div = "//"    
else : #platform = win32
    path_div = "\\"


CURRENT_PATH = sys.path[0]
# In case the scripts were run from current directory
CURRENT_PATH_SCRIPTS = path_div +  ".." + path_div + "common"
# this is the scripts local path, from where the program was called
sys.path.append(CURRENT_PATH+CURRENT_PATH_SCRIPTS)

import configparser
from dmpu_util_helper import *
import sys


# Parse given test configuration file and return test attributes as dictionary
def parse_config_file (config, log_file):
    local_dict = {}
    section_name = "OPT_IMAGE_CFG"

    if not config.has_section(section_name):
        log_sync(log_file, "section " + section_name + " wasn't found in cfg file\n")
        return None

    # opt image file name
    if config.has_option(section_name, 'opt_image-filename'): 
        optImageFilePath_str = config.get(section_name, 'opt_image-filename')
        if os.path.isfile(optImageFilePath_str):
            local_dict['opt_image-filename'] = optImageFilePath_str
            log_sync(log_file,"opt_image-filename: " + str(local_dict['opt_image-filename']) + "\n")    
        else:
            log_sync(log_file,"opt_image-filename: " + str(optImageFilePath_str) + " File Not Found !!! \n")      
            return None              
    else:
        log_sync(log_file,"opt exec image not provided \n")
        return None


    # Get dmpu_data-filename
    if config.has_option(section_name, 'dmpu_data-filename'): 
        dmpuDataFilePath_str = config.get(section_name, 'dmpu_data-filename')
        if os.path.isfile(optImageFilePath_str):
            local_dict['dmpu_data-filename'] = dmpuDataFilePath_str
            log_sync(log_file,"dmpu_data-filename: " + str(local_dict['dmpu_data-filename']) + "\n")    
        else:
            log_sync(log_file,"dmpu_data-filename: " + str(dmpuDataFilePath_str) + " File Not Found !!! \n")      
            return None              
    else:
        log_sync(log_file,"dmpu data file not provided \n")
        return None


    # Get the output file name
    if config.has_option(section_name, 'opt_combined-filename'): 
        local_dict['opt_combined-filename'] = config.get(section_name, 'opt_combined-filename')
        log_sync(log_file,"opt_combined-filename: " + str(local_dict['opt_combined-filename']) + "\n")        
    else:
        log_sync(log_file,"Output File name not provided !!! \n")
        return None 

    return local_dict

# Parse script parameters
def parse_shell_arguments ():
    len_arg =  len(sys.argv)
    if len_arg < 2:
        print_sync("len " + str(len_arg) + " invalid. Usage:" + sys.argv[0] + "<test configuration file>\n")
        for i in range(1,len_arg):
            print_sync("i " + str(i) + " arg " + sys.argv[i] + "\n")
        sys.exit(1)
    config_fname = sys.argv[1]
    if len_arg == 3:
        log_fname = sys.argv[2]
    else:
        log_fname = "asset_prov.log"
    return config_fname, log_fname


# close files and exit script
def exit_main_func(log_file, config_file, rc):
    log_file.close()
    config_file.close()
    sys.exit(rc)


def main():

    config_fname, log_fname = parse_shell_arguments()
    log_file = create_log_file(log_fname)
    print_and_log(log_file, str(datetime.now()) + ": Combining opt executable and data file utility started (Logging to " + log_fname + ")\n")

    print("Processing config file \n")

    try:
        config_file = open(config_fname, 'r')
    except IOError as e:
        print_and_log(log_file,"Failed opening " + config_fname + " (" + e.strerror + ")\n")
        log_file.close()
        sys.exit(e.errno)

    config = configparser.ConfigParser()
    config.read(config_fname)
    data_dict = {}

    data_dict = parse_config_file(config, log_file)

    if (data_dict != None):
        print("Parser Done \n")

        fo = open(data_dict['opt_image-filename'], "r+")
        # truncate to 0x7000 bytes
        fo.truncate(28672)
        fo.close()

        open(data_dict['opt_combined-filename'],'wb').write(open(data_dict['opt_image-filename'],'rb').read()+open(data_dict['dmpu_data-filename'],'rb').read())
  

        print_and_log(log_file, "**** truncate completed successfully ****\n")
        exit_main_func(log_file, config_file, 0)       


    else:
        print_and_log(log_file, "**** Invalid config file ****\n")
        exit_main_func(log_file, config_file, 1)



#############################
if __name__ == "__main__":
    main()



