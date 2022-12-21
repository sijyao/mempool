// Copyright 2022 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
\
<% def array_to_cstr(array):
    out = '{'
    i = 0
    out += '\n'
    for a in array:
        out += '{}, '.format(a)
        i += 1
        if i % 32 == 0:
            out += '\n'
    out = out[:-2] + '}'
    return out
%> \

#define N_TX (${nb_tx})
#define N_RX (${nb_rx})
#define N_SAMPLES (${nb_samples})

int16_t PilotRX[${2*nb_rx*nb_samples}] = ${array_to_cstr(pilot_rx)};

int16_t PilotTX[${2*nb_tx*nb_samples}] = ${array_to_cstr(pilot_tx)};

int16_t HEST[${2*nb_rx*nb_tx*nb_samples}] = ${array_to_cstr(Hest)};
