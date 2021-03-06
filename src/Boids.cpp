//
//  Boids.cpp
//  curiositas2017
//
//  Created by FatDazz_mac on 04/04/2017.
//
//
#pragma once
#include "Boids.h"
#include "BoidsThread.h"
#include "ofMain.h"
#include "Constant.h"



Boid2d::Boid2d(Flock2d * _flock){
    this->flockPtr = _flock;
}
Boid2d * Boid2d::setFlock(Flock2d * flock) {
    this->flockPtr = flock;
    return this;
}
void Boid2d::bounds() {
    switch (flockPtr->boundmode) {
        case 0: // CLAMP
            if (position.x < flockPtr->minX) {
                position.x = flockPtr->minX;
                velocite.x = -velocite.x;
            }
            if (position.x > flockPtr->maxX) {
                position.x = flockPtr->maxX;
                velocite.x = -velocite.x;
            }
            if (position.y < flockPtr->minY) {
                position.y = flockPtr->minY;
                velocite.y = -velocite.y;
            }
            if (position.y > flockPtr->maxY) {
                position.y = flockPtr->maxY;
                velocite.y = -velocite.y;
            }
            break;
        case 1: // WRAP
            if (position.x < flockPtr->minX) {
                position.x+= flockPtr->boundsWidth;
            }
            if (position.x > flockPtr->maxX) {
                position.x -= flockPtr->boundsWidth;
            }
            if (position.y < flockPtr->minY) {
                position.y += flockPtr->boundsHeight;
            }
            if (position.y > flockPtr->maxY) {
                position.y -= flockPtr->boundsHeight;
            }
            break;
    }
    
}
void Boid2d::update(vector<Boid2d *>  *otherBoids, vector<vector<ofVec2f>> *_fieldVector){
    
    if(!active) {
        return;
    }
    acceleration.x = 0;
    acceleration.y = 0;
    float *vec = new float[2];
    vec[0] = 0.0f;
    vec[1] = 0.0f;
    flockfullNew(vec, otherBoids);
    
    acceleration.x += vec[0];// *amount;
    acceleration.y += vec[1];// *amount;
    delete [] vec;
    
    if (flockPtr->isVectorField) {
      // std::cout << _fieldVector[0].size() << "\n";
        acceleration = acceleration+ (_fieldVector->at(
                                                       trunc(max(0.0f, min((float)this->position.x/divGrad_width,(float)(win_width/divGrad_width)-1))))
                                      .at(
                                          trunc(max(0.0f, min((float) this->position.y/divGrad_height,(float)(win_height/divGrad_height)-1)))));
        
        
    }
    
    float distMaxForce = ABS(acceleration.x) + ABS(acceleration.y);
    if (distMaxForce > maxForce) {
        distMaxForce = maxForce / distMaxForce;
        acceleration *= distMaxForce;
    }
    velocite += acceleration;
    // limit speed
    float distMaxSpeed = ABS(velocite.x) + ABS(velocite.y);
    if (distMaxSpeed > maxSpeed) {
        distMaxSpeed = maxSpeed / distMaxSpeed;
        velocite *= distMaxSpeed;
    }
    position += velocite * flockPtr->dt;
    
    bounds();
    
}
float* Boid2d::flockfullNew(float *vec, vector<Boid2d*> *otherBoids) {
    
    float *sep = new float[2];
    float *ali = new float[2];
    float *coh = new float[2];
    float *attrForce = new float[2];
    
    for (int i=0; i<2; i++) {
        sep[i] = 0.0f;
        ali[i] = 0.0f;
        coh[i] = 0.0f;
        attrForce[i] = 0.0f;
    }
    
    
    int countsep = 0, countali = 0, countcoh = 0;
    float invD = 0;
    
    for (vector<Boid2d*>::iterator i =otherBoids->begin(); i != otherBoids->end(); i++) {
        Boid2d * other =  i.operator*();
            
            float separatedist = other->distSeparationGroup;
            float aligndist = other->distAlignGroup;
            float cohesiondist = other->distCohesionGroup;
            
            float dx = other->position.x - position.x;
            float dy = other->position.y - position.y;
            float d = ABS(dx) + ABS(dy);
            if (d <= 1e-7)
                continue;
            invD = 1.f / d;
            if (d < separatedist) { // sep
                countsep++;
                foncSep(dx, dy, invD, other,sep);
            }
            if (d < cohesiondist) { // coh
                countcoh++;
                foncCohe(d, 1.0 , other, coh);
            }
            if (d < aligndist) { // ali
                countali++;
                foncAlig(other, ali);
            }
        
    }
    
    
    if (countsep > 0) {
        const float invForSep = 1 / (float) countsep; // faire invForsep une moyenne
        sep[0] *= invForSep;
        sep[1] *= invForSep;
    }
    if (countali > 0) {
        const float invForAli = 1 / (float) countali; // final float invForAli = 1f / (float) countali;
        ali[0] *= invForAli;
        ali[1] *= invForAli;
    }
    if (countcoh > 0) {
        const float invForCoh = 1 / (float) countcoh;
        coh[0] *= invForCoh;
        coh[1] *= invForCoh;
        coh = steer(coh, 1);
    }
    
    vec[0] = sep[0] + ali[0] + coh[0] + attrForce[0];
    vec[1] = sep[1] + ali[1] + coh[1] + attrForce[1];
    const float d = ABS(vec[0]) + ABS(vec[1]);
    if (d > 0) {
        float invDist = flockPtr->dt / d;
        vec[0] *= invDist;
        vec[1] *= invDist;
    }
    vec[0] *= flockPtr->dt;
    vec[1] *= flockPtr->dt;
    delete[] sep;
    delete[] ali;
    delete[] coh;
    delete[] attrForce;
    return vec;
}
float * Boid2d::steer(float* target, float amount){ //, float *steervec) {
    
    //	float steer[] = {0.f, 0.f}; //new float[2];
    //	float *dir =new float[2];
    float dir[2] = {0.f,0.f};
    //	dir[0] = 0.0f;
    //	dir[1] = 0.0f;
    
    
    dir[0] = target[0] - position.x;
    dir[1] = target[1] - position.y;
    float d = ABS(dir[0]) + ABS(dir[1]);
    
    if (d > 2) {
        float invDist = 1.f / d;
        dir[0] *= invDist;
        dir[1] *= invDist;
        // steer, desired - vel
        target[0] = dir[0] - velocite.x;
        target[1] = dir[1] - velocite.y;
        float steerLen = ABS(target[0]) + ABS(target[1]);
        if (steerLen > 0) {
            float invSteerLen = amount / steerLen;// 1f / steerLen;
            target[0] *= invSteerLen;
            target[1] *= invSteerLen;
        }
    }
    
    //	delete [] dir;
    
    return target;
    
}
float * Boid2d::foncSep(const float dx, const float dy, const float invD, Boid2d *other, float *sep){
    sep[0] -= dx * invD * other->separateGroup ;
    sep[1] -= dy * invD * other->separateGroup ;
    return sep;
}
float * Boid2d::foncCohe(const float d, const float variable,Boid2d *other, float *coh){
    coh[0] += other->position.x * other->cohesionGroup * d/variable;
    coh[1] += other->position.y * other->cohesionGroup * d/variable;
    return coh;
}
float * Boid2d::foncAlig(Boid2d *other, float *ali){
    ali[0] += other->velocite.x * other->alignGroup;
    ali[1] += other->velocite.y * other->alignGroup;
    return ali;
}
void Boid2d::returnInitial(){
    if (this->position == this->positionInitiale) {
        return;
    }
    
}


