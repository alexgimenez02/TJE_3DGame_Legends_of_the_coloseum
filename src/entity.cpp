#include "entity.h"

//Entity

void Entity::addChild(Entity* ent)
{
}

void Entity::removeChild(Entity* ent)
{
}

//get the global transformation of this object (not the relative to the parent)
//this function uses recursivity to crawl the tree upwards
Matrix44 Entity::getGlobalMatrix()
{
    if (parent) //if I have a parent, ask his global and concatenate
        return model * parent->getGlobalMatrix();
    return model; //otherwise just return my model as global
}

void Entity::destroy()
{
}

Vector3 Entity::getPosition()
{
    return Vector3();
}





//EntityMesh
void EntityMesh::render()
{
    assert(mesh != NULL, "mesh in EntityMesh::render() was null");

    if (!shader) return;
    //get the last camera that was activated
    Camera* camera = Camera::current;
    Matrix44 model = this->model;

    //enable shader and pass uniforms
    shader->enable();
    shader->setUniform("u_model", model);
    shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
    shader->setTexture("u_texture", texture,0);

    //render the mesh using the shader
    // 
    // 
    /*for (int i = 0; i < children.size(); i++)
        children[i]->render();  //repeat for every child
    */
    mesh->render(GL_TRIANGLES);

    //disable the shader after finishing rendering
    shader->disable();
}

void EntityMesh::update(float elapsed_time)
{
}



//EntityMap

void EntityMap::render()
{
}

void EntityMap::update(float elapsed_time)
{
}
