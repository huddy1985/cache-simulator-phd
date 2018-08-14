INCLUDE := include
export INCLUDE

CC := gcc
export CC

OBJSDIR := $(abspath $(lastword $(MAKEFILE_LIST)))
export OBJSDIR