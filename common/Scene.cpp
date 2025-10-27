
#include "Scene.hpp"

Scene::~Scene(){
    for (int i=0;i<sceneObjects.size();i++)
    {
        delete sceneObjects[i];
    }
    sceneObjects.clear();
}


// Scene.cpp
void Scene::render(Camera* camera) {
    for (std::size_t i = 0; i < sceneObjects.size(); ++i) {
        sceneObjects[i]->getShader()->bind();   // MUST be present
        sceneObjects[i]->render(camera);
    }
}

void Scene::addObject(Object *object){
    sceneObjects.push_back(object);
    
}

