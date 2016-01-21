#!/usr/bin/env sh

set -e

./scripts/pjac $1 && viua-asm $1.asm && viua-vm a.out
