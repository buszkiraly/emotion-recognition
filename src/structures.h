#ifndef STRUCTURES_H
#define STRUCTURES_H


struct params{
    int     id;
    bool     inUse;
    int     xShift;
    int     yShift;
    double  modelScale;
    double  modelRot;
    int     modelIterations;
    bool    fit;
    bool    memo;
};

#endif // STRUCTURES_H


