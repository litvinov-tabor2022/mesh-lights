#ifndef MESH_LIGHTS_NODE_H
#define MESH_LIGHTS_NODE_H

#include "StateManager.h"
#include "painlessMesh.h"
#include "devices/NfcManager.h"

class Node {
public:
    Node() {}

    virtual bool init(painlessMesh *mesh, Scheduler *scheduler, NfcManager *nfcManager, StateManager *stateManager, int statusLedPin) {
        this->mesh_ = mesh;
        this->scheduler_ = scheduler;
        this->stateManager_ = stateManager;
        this->statusLedPin_ = statusLedPin;
        this->nfcManager = nfcManager;
        return true;
    }

    painlessMesh *mesh_;
    Scheduler *scheduler_;
    StateManager *stateManager_;
    NfcManager *nfcManager;
    int statusLedPin_;
private:
    int PWM1_Ch = 0;
    int PWM1_Res = 8;
    int PWM1_Freq = 1000;
    int PWM1_DutyCycle = 0;
};


#endif //MESH_LIGHTS_NODE_H
