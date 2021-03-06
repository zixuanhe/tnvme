/*
 * Copyright (c) 2011, Intel Corporation.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "createIOCQ.h"
#include "globals.h"
#include "../Utils/buffers.h"

SharedCreateIOCQPtr CreateIOCQ::NullCreateIOCQPtr;
const uint8_t CreateIOCQ::Opcode = 0x05;


CreateIOCQ::CreateIOCQ() : Cmd(Trackable::OBJ_CREATEIOCQ)
{
    Cmd::Init(Opcode, DATADIR_FROM_DEVICE, 64);

    // No cmd should ever be created which violates these masking possibilities
    send_64b_bitmask allowPrpMask = (send_64b_bitmask)
        (MASK_PRP1_PAGE | MASK_PRP1_LIST);
    SetPrpAllowed(allowPrpMask);
}


CreateIOCQ::~CreateIOCQ()
{
}


void
CreateIOCQ::Init(const SharedIOCQPtr iocq)
{
    // Setup the PRP buffer based upon contig or non-contig memory
    int prpField = MASK_PRP1_PAGE;
    if (iocq->GetIsContig() == false)
        prpField |= MASK_PRP1_LIST;
    SetPrpBuffer((send_64b_bitmask)prpField, iocq->GetQBuffer(),
        iocq->GetQSize());

    {   // Handle DWORD 10
        uint32_t dword10 = GetDword(10);

        // Handle q size
        dword10 &= ~0xffff0000;
        dword10 |= (((uint32_t)(iocq->GetNumEntries() - 1)) << 16);

        // Handle Q ID
        dword10 &= ~0x0000ffff;
        dword10 |= (uint32_t)iocq->GetQId();

        SetDword(dword10, 10);
    }   // Handle DWORD 10

    {   // Handle DWORD 11
        uint32_t dword11 = GetDword(11);

        // Handle the PC bit
        if (iocq->GetIsContig())
            dword11 |= 0x00000001;
        else
            dword11 &= ~0x00000001;

        // Handle IRQ support
        if (iocq->GetIrqEnabled()) {
            dword11 |=  0x00000002;
            dword11 &= ~0xffff0000;    // clear it, then set it

            enum nvme_irq_type irq;
            uint16_t numIrqs;
            if (gCtrlrConfig->GetIrqScheme(irq, numIrqs) == false)
                throw FrmwkEx(HERE, "Unable to retrieve current IRQ scheme");

            switch (irq) {
            case INT_MSI_MULTI:
            case INT_MSIX:
                dword11 |= (((uint32_t)iocq->GetIrqVector()) << 16);
                break;
            case INT_MSI_SINGLE:
            case INT_NONE:
                ;   // Required to be zero
                break;
            default:
                throw FrmwkEx(HERE, "Unsupported IRQ scheme, what to do?");
            }
        } else {
            dword11 &= ~0x00000002;
            dword11 &= ~0xffff0000;
        }

        SetDword(dword11, 11);
    }   // Handle DWORD 11
}

void
CreateIOCQ::InitInvalidPrpOffset(const SharedIOCQPtr iocq)
{
    // Setup the PRP buffer based upon contig or non-contig memory
//    int prpField = MASK_PRP1_PAGE;
//    if (iocq->GetIsContig() == false)
//        prpField |= MASK_PRP1_LIST;
//    SetPrpBuffer((send_64b_bitmask)prpField, iocq->GetQBuffer(),
//        iocq->GetQSize());

    {   // Handle DWORD 10
        uint32_t dword10 = GetDword(10);

        // Handle q size
        dword10 &= ~0xffff0000;
        dword10 |= (((uint32_t)(iocq->GetNumEntries() - 1)) << 16);

        // Handle Q ID
        dword10 &= ~0x0000ffff;
        dword10 |= (uint32_t)iocq->GetQId();

        SetDword(dword10, 10);
    }   // Handle DWORD 10

    {   // Handle DWORD 11
        uint32_t dword11 = GetDword(11);

        // Handle the PC bit
        if (iocq->GetIsContig())
            dword11 |= 0x00000001;
        else
            dword11 &= ~0x00000001;

        // Handle IRQ support
        if (iocq->GetIrqEnabled()) {
            dword11 |=  0x00000002;
            dword11 &= ~0xffff0000;    // clear it, then set it

            enum nvme_irq_type irq;
            uint16_t numIrqs;
            if (gCtrlrConfig->GetIrqScheme(irq, numIrqs) == false)
                throw FrmwkEx(HERE, "Unable to retrieve current IRQ scheme");

            switch (irq) {
            case INT_MSI_MULTI:
            case INT_MSIX:
                dword11 |= (((uint32_t)iocq->GetIrqVector()) << 16);
                break;
            case INT_MSI_SINGLE:
            case INT_NONE:
                ;   // Required to be zero
                break;
            default:
                throw FrmwkEx(HERE, "Unsupported IRQ scheme, what to do?");
            }
        } else {
            dword11 &= ~0x00000002;
            dword11 &= ~0xffff0000;
        }

        SetDword(dword11, 11);
    }   // Handle DWORD 11
}


