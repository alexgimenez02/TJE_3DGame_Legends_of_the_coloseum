#include "entity.h"
#include "game.h"

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

string EntityMesh::toString()
{
    string ret = "";
    ret += "Entity Name: " + name;
    ret += "\nHas Mesh: ";
    if (mesh) ret += "Yes";
    else ret += "No";
    ret += "\nHas Texture: ";
    if (texture) ret += "Yes";
    else ret += "No";
    ret += "\nWith position: ";
    ret += "(";
    ret += pos.x;
    ret += ",";
    ret += pos.y;
    ret += ",";
    ret += pos.z;
    ret += ")";
    ret += "\nWith model matrix: ";
    ret += "\n";
    ret += model._11;
    ret += model._12;
    ret += model._13;
    ret += model._14;
    ret += "\n";
    ret += model._21;
    ret += model._22;
    ret += model._23;
    ret += model._24;
    ret += "\n";
    ret += model._31;
    ret += model._32;
    ret += model._33;
    ret += model._34;
    ret += "\n";
    ret += model._41;
    ret += model._42;
    ret += model._43;
    ret += model._44;
    ret += "--------------------------------------------------------------------";
    return ret;
}



//EntityMap

void EntityMap::render()
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

        //float padding = 10000.0f;
        
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
