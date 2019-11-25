#!/bin/bash
#
################################################################################
# usage: obfuscate.sh [OPTIONS] <input_bitcode> <output_bitcode>
#
# Obfuscates bitcode with options of Virtualization (VIRT), Instruction 
# Substitution (SUB), Bogus Control Flow (BCF), and Control Flow Flattening
# (FLA).
################################################################################


####################
### Command line ###
####################

obfs_args=""
obfs_all=false

while getopts "o:a" OPTION
do
  case $OPTION in
    o)
      obfs_args="$OPTARG"
      ;;
    a)
      obfs_all=true
      ;;
  esac
done

shift $(( OPTIND - 1 ))

# Parse arguments
if [ $# -lt 2 ]; then
  echo "usage: obfuscate.sh [OPTIONS] <input_bitcode> <output_bitcode>"
  echo "  OPTIONS:"
  echo "    -o <obfs_technique>: obfuscate protections"
  echo "    -a <obfs_all>: obfuscate protections"
  exit 1
fi

input_bitcode=$(realpath $1)
output_bitcode=$(realpath $2)


#################
### Variables ###
#################

VIRT_LIB=$SIP_TOOLS/sc-virt/build/lib

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
CUSTOM_PASSES_LIB=$DIR/passes/build/lib


#################
### Functions ###
#################

run_llvm_obfs () {
  input_bitcode=$1
  output_bitcode=$2
  pass_flags=$3

  opt-7 -load $CUSTOM_PASSES_LIB/libObfsPass.so $input_bitcode -o $output_bitcode $pass_flags
  if [ $? -ne 0 ]; then
    echo Failed obfuscation
    exit 1
  fi
}


###########
### RUN ###
###########

if [ $input_bitcode != $output_bitcode ]; then
  cp $input_bitcode $output_bitcode
fi

# If obfuscate all flag, annotate each function for obfuscation
if $obfs_all; then
  opt-7 -load $CUSTOM_PASSES_LIB/libPrepPass.so $output_bitcode -o $output_bitcode -prep -obfs $obfs_args
  if [ $? -ne 0 ]; then
    echo Failed obfs prep
    exit 1
  fi
fi

# check obfuscation techniques
obfs_args="$(echo $obfs_args | tr - ' ')"
for obfs in $obfs_args; do
  if [[ $obfs == "SUB"* ]]; then
    obfs_flag="-substitution"
    pass_opt=${obfs:3}
    if [[ $pass_opt != "" ]]; then
      # Number of passes
      if [[ $pass_opt =~ ^[0-9]+$ ]]; then
        obfs_flag="$obfs_flag -sub_loop=$pass_opt"
      # Bad Options
      else
        echo Bad number of instruction substitution passes
        exit 1
      fi
    fi

    # Call Obfs
    run_llvm_obfs $output_bitcode $output_bitcode "$obfs_flag"
  elif [[ $obfs == "BCF"* ]]; then
    obfs_flag="-boguscf"
    pass_opt=${obfs:3}
    if [[ $pass_opt != "" ]]; then
      # Number of passes
      if [[ $pass_opt =~ ^[0-9]+ ]]; then
        obfs_flag="$obfs_flag -bcf_loop=$BASH_REMATCH"
        pass_opt=${pass_opt:${#BASH_REMATCH}}
      fi
      # Block percentage probability
      if [[ $pass_opt =~ ^p[0-9]+$ ]]; then
        prob=${pass_opt:1}
        if [ $prob -lt 1 ] || [ $prob -gt 100 ]; then
          echo Bad BCF probability
          exit 1
        fi
        obfs_flag="$obfs_flag -bcf_prob=$prob"
        pass_opt=""
      fi
      # Check for bad options
      if [[ $pass_opt != "" ]]; then
        echo Bad BCF options
        exit 1
      fi
    fi

    # Call Obfs
    run_llvm_obfs $output_bitcode $output_bitcode "$obfs_flag"
  elif [[ $obfs == "FLA"* ]]; then
    obfs_flag=""
    pass_opt=${obfs:3}
    if [[ $pass_opt != "" ]]; then
      if [[ $pass_opt == "s"* ]]; then
        obfs_flag="$obfs_flag -splitbbl"
        pass_opt=${pass_opt:1}
      fi
      if [[ $pass_opt =~ ^[0-9]+$ ]]; then
        obfs_flag="$obfs_flag -split_num=$pass_opt"
        pass_opt=""
      fi
      if [[ $pass_opt != "" ]]; then
        echo Bad FLA options
        exit 1
      fi
    fi
    obfs_flag="$obfs_flag -flattening"

    # Call Obfs
    run_llvm_obfs $output_bitcode $output_bitcode "$obfs_flag"
  else
    echo "Bad value of <obfs_technique>"
    exit 1
  fi
done
