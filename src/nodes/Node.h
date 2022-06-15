#ifndef MESH_LIGHTS_NODE_H
#define MESH_LIGHTS_NODE_H

#include "StateManager.h"
#include "painlessMesh.h"

class Node {
public:
    Node() {}

    virtual bool init(painlessMesh *mesh, Scheduler *scheduler, StateManager *stateManager) {
        this->mesh_ = mesh;
        this->scheduler_ = scheduler;
        this->stateManager_ = stateManager;
        return true;
    }

    virtual void begin() = 0;

    painlessMesh *mesh_;
    Scheduler *scheduler_;
    StateManager *stateManager_;
};


#endif //MESH_LIGHTS_NODE_H
