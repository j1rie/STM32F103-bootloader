##
## This file is part of the libopencm3 project.
##
## Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
##
## This library is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this library.  If not, see <http://www.gnu.org/licenses/>.
##

BINARY = usbdfu
CSTD = -std=gnu99

LDSCRIPT = F103C8.ld

include target.mk

ifeq ($(Platform), Blue)
DEFS += -DBlue
endif
ifeq ($(Platform), Blue_noHW)
DEFS += -DBlue_noHW
endif
ifeq ($(Platform), Red)
DEFS += -DRed
endif
ifeq ($(Platform), Red_noHW)
DEFS += -DRed_noHW
endif
ifeq ($(Platform), Dev)
DEFS += -DDev
endif
ifeq ($(Platform), blueDev)
DEFS += -DblueDev
endif
ifeq ($(Platform), blackDev)
DEFS += -DblackDev
endif
ifeq ($(Platform), MapleMini)
DEFS += -DMapleMini
endif
ifeq ($(Platform), MapleMini_2k)
DEFS += -DMapleMini_2k
endif
