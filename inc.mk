INCLUDE := include
export INCLUDE

CC := gcc
export CC

OBJSDIR := $(abspath $(lastword $(MAKEFILE_LIST)))
export OBJSDIR

INCLUSIVE_SRC_DIR := $(CURDIR)/src/inclusive
export INCLUSIVE_SRC_DIR

CFG_PARSER := $(CURDIR)/cfg-parser
export CFG_PARSER
