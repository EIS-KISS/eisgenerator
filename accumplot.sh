#!/bin/sh
#
# eisgenerator - a shared libary and application to generate EIS spectra
# Copyright (C) 2022-2024 Carl Philipp Klemm <carl@uvos.xyz>
#
# This file is part of eisgenerator.
#
# eisgenerator is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# eisgenerator is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with eisgenerator.  If not, see <http://www.gnu.org/licenses/>.
#

SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]:-$0}"; )" &> /dev/null && pwd 2> /dev/null; )";

gnuplot -p -e "dir='$1'" $SCRIPT_DIR/density.p

