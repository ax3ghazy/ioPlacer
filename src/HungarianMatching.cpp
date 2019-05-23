////////////////////////////////////////////////////////////////////////////////
// Authors: Vitor Bandeira, Mateus Fogaça, Eder Matheus Monteiro e Isadora
// Oliveira
//          (Advisor: Ricardo Reis)
//
// BSD 3-Clause License
//
// Copyright (c) 2019, Federal University of Rio Grande do Sul (UFRGS)
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
////////////////////////////////////////////////////////////////////////////////

#include "HungarianMatching.h"

HungarianMatching::HungarianMatching(Section_t& section) {
        _netlist = &section.net;
        _slots = &section.sv;
        _numIOPins = _netlist->numIOPins();
        _numSlots = _slots->size();
}

void HungarianMatching::run() {
        createMatrix();
        _hungarianSolver.solve(_hungarianMatrix);
}

void HungarianMatching::createMatrix() {
        _hungarianMatrix = Matrix<DBU>(_numSlots, _numIOPins);
#pragma omp parallel for
        for (unsigned i = 0; i < _numSlots; ++i) {
                unsigned pinIndex = 0;
                Coordinate newPos = (*_slots)[i].pos;
                _netlist->forEachIOPin([&](unsigned idx, IOPin& ioPin) {
                        DBU hpwl = _netlist->computeIONetHPWL(idx, newPos);
                        _hungarianMatrix(i, pinIndex) = hpwl;
                        pinIndex++;
                });
        }
}

inline bool samePos(Coordinate& a, Coordinate& b) {
        return (a.getX() == b.getX() && a.getY() == b.getY());
}

void HungarianMatching::getFinalAssignment(std::vector<IOPin>& assigmentVector,
                                           slotVector_t& slots) {
        slotVector_t& matrixSlots = *_slots;
        size_t rows = _hungarianMatrix.rows();
        size_t col = 0;
        _netlist->forEachIOPin([&](unsigned idx, IOPin& ioPin) {
                for (size_t row = 0; row < rows; row++) {
                        if (_hungarianMatrix(row, col) != 0) {
                                continue;
                        }
                        ioPin.setPos(matrixSlots[row].pos);
                        assigmentVector.push_back(ioPin);
                        Coordinate sPos = matrixSlots[row].pos;
                        for (unsigned i = 0; i < slots.size(); i++) {
                                if (samePos(slots[i].pos, sPos)) {
                                        slots[i].used = true;
                                        break;
                                }
                        }
                }
                col++;
        });
}
