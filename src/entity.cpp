#include "entity.h"
#include "game.h"

//Entity

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
    this->~Entity();
}





//EntityMesh
void EntityMesh::render() //Render for the normal entities
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
    mesh->render(GL_TRIANGLES);

    //disable the shader after finishing rendering
    shader->disable();
}

void EntityMesh::update(float elapsed_time)
{

}

//EntityMap

void EntityMap::render() //Render for the map entity
{
    if (shader)
    {
        Camera * camera = Game::instance->camera;
        shader->enable();

        //upload uniforms
        shader->setUniform("u_color", Vector4(1, 1, 1, 1));
        shader->setUniform("u_viewprojection",camera->viewprojection_matrix);
        shader->setUniform("u_texture", texture, 0);
        shader->setUniform("u_time", time);
        shader->setUniform("u_tex_tiling", 1.0f);
        
        model.setTranslation(camera->eye.x, camera->eye.y - 10.0f, camera->eye.z);
        shader->setUniform("u_model", model);


        //do the draw call
        mesh->render(GL_TRIANGLES);

        //disable shader
        shader->disable();
    }
}

void EntityMap::update(float elapsed_time)
{
}
