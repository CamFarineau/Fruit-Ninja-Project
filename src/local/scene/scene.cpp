/*****************************************************************************\
 * 			IMI Projet - 4ETI
 *          Juin 2016 @CPE Lyon
 *			 --------------
 *	   Author :  Camille Farineau / Lucas Maigre / Prince Ngattai-Lam
 *
 *       ///   scene.cpp  ///
 *  Ce fichier contient tous les éléments relatifs à la scene 3D
 *  Il utilise beaucoup de classe et fonctions definies dans le dossier lib
 *  Il contient toutes les déclarations nécessaires, notamment celle de la 
 *  classe scene qui gère la scène 3D
 *
\*****************************************************************************/

/*****************************************************************************\
 * Inclusions et libraires nécessaires
\*****************************************************************************/
#include <GL/glew.h>
#include <stdio.h>     
#include <stdlib.h>    
#include <time.h>
#include <string>
#include <sstream>
#include <fstream>
#include <cmath>

#include "scene.hpp"
#include "../../lib/common/error_handling.hpp"
#include "../../lib/opengl/glutils.hpp"
#include "../../lib/perlin/perlin.hpp"
#include "../../lib/interface/camera_matrices.hpp"
#include "../interface/myWidgetGL.hpp"
#include "../../lib/mesh/mesh_io.hpp"
#include "../interface/myWindow.hpp"
#include "../interface/myWidgetGL.hpp"

using namespace cpe;

/*****************************************************************************\
 * Variables Globales
\*****************************************************************************/
// Score de l'utilisateur
int score = 000;
// Meilleur score (récupère dans le fichier score.txt)
int score_best =000;
// Nombre de fruits présent à l'écran
int nb_fruits_pres = 0;
// Nombre de "points" présent à l'écran
int nb_points_pres = 0;
// Constante symbolisant la gravité
vec2 const gravity={0.0f,9.81f};
// Damping (faible donc pas de résistance)
float damping = 0.001f;
// Pas d'intégration
float dt = 0.35f;
// Coefficient de rebond (fixe à 1.O pas d'amortissement)
float bounce_coeff = 1.0f;
// Valeur seuil pour la vitesse suivant x et y
float epsilon_x = 0.2f;
float epsilon_y = 0.2f;
// Temps de rafraichissement pour créer un nouveau fruit
// Fixé au départ puis diminue au fur et à mesure pour augmenter difficultés
int temps_raf = 40;
// Nombre de coeurs
int nb_coeurs=3;
// Flags pour savoir dans quelle partie du jeu on est
// Flag pour lancer jeu avec vies
bool begin_game_life = false;
// Flag pour lancer jeu avec temps
bool begin_game_time = false;
// Flag pour l'écran d'accueil
bool initialisation_accueil = false;
// Flag pour fin du jeu
bool end_game = false;
// Couleur Side
int Couleur_Side =0;

// Déclarations des tableaux des instances de classes
// Tableau des fruits d'une longueur de NB_MAX
// Des qu'on arrive à la fin du tableau on recommence au début
Fruit fruits[NB_MAX];
// Tableau de fruit pour la page d'acceuil
Fruit fruit_accueil[2];
// Tableau des "points"
Point points[NB_MAX];
// Tableau des coeurs
Coeur coeurs[NB_COEUR];

// Vector pour l'affichage du score
std::vector<vec2> chiffres_score;
// Vector pour l'affichage du temps
std::vector<vec2> chiffres_temps;
// Vector pour l'affichage du meilleur score
std::vector<vec2> chiffres_best_score;
// Vector pour l'affichage du score final
std::vector<vec2> chiffres_score_final;


///// COLOR TRACKING //////

int STEP_MIN =5;
int STEP_MAX = 100;

int NB_PIXEL=5;
//int morph_size = 1;

bool choix_couleur = true;

// Position of the object we overlay
// Color tracked and our tolerance towards it
int h_r = 0, s_r = 0, v_r = 0, tolerance_r = 25;
int h_l = 0, s_l = 0, v_l = 0, tolerance_l = 25;

//Current image to analyse
cv::Mat image;

// Position of the right color
cpe::vec2 objectPos_r = cpe::vec2(-1, -1);

// Position of the left color
cpe::vec2 objectPos_l = cpe::vec2(-1, -1);
// Key for keyboard event
char key;

// Number of tracked pixels
int nbPixels;
// Next position of the object we overlay
cpe::vec2 objectNextPos_r;
cpe::vec2 objectNextPos_l;

/*****************************************************************************\
 * Define
\*****************************************************************************/

#define COLOR_RIGHT 0
#define COLOR_LEFT 1

// Maths methods
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define abs(x) ((x) > 0 ? (x) : -(x))
#define sign(x) ((x) > 0 ? 1 : -1)



/*****************************************************************************\
 * Méthode load_scene
 * Cette fonction se charge d'appeler toutes les fonctions de load
 * de lancer la webcam, lire meilleur score etc.
\*****************************************************************************/
void scene::load_scene()
{
    load_common_data();
    start_webcam();

    //*****************************************//
    // Background                              //
    //*****************************************//

    mesh_camera.add_vertex( { -1.0f , -1.0f , 0.5f } );
    mesh_camera.add_vertex( { -1.0f , +1.0f , 0.5f } );
    mesh_camera.add_vertex( { +1.0f , +1.0f , 0.5f } );
    mesh_camera.add_vertex( { +1.0f , -1.0f , 0.5f } );

    mesh_camera.add_texture_coord( { 1.0f , 1.0f } );
    mesh_camera.add_texture_coord( { 1.0f , 0.0f } );
    mesh_camera.add_texture_coord( { 0.0f , 0.0f } );
    mesh_camera.add_texture_coord( { 0.0f , 1.0f } );

    mesh_camera.add_triangle_index( { 0,2,1 } );
    mesh_camera.add_triangle_index( { 0,3,2 } );

    mesh_camera.fill_empty_field_by_default();
    mesh_camera_opengl.fill_vbo(mesh_camera);


    load_background();
    load_fruits();
    load_coeurs();
    load_sword();
    load_point();
    load_start();
    load_best();
    load_final();
    lecture_score();
    load_chiffres();
}

/*****************************************************************************\
 * Méthode load_background
 * Cette fonction se charge de gérer l'affichage du background à partir d'une
 * texture
\*****************************************************************************/
void scene::load_background()
{
	// Lecture du shader
    shader_background = read_shader("shaders/shader_cube.vert",
                                    "shaders/shader_cube.frag"); PRINT_OPENGL_ERROR();
    // Lecture de la texture
    texture_background = load_texture_file("data/background.jpg");

    // Création d'une mesh pour y placer la texture
    mesh_background.add_vertex( { -1.0f , -1.0f , -1.0f } );
    mesh_background.add_vertex( { -1.0f , +1.0f , -1.0f } );
    mesh_background.add_vertex( { +1.0f , +1.0f , -1.0f } );
    mesh_background.add_vertex( { +1.0f , -1.0f , -1.0f } );

    mesh_background.add_texture_coord( { 1.0f , 1.0f } );
    mesh_background.add_texture_coord( { 1.0f , 0.0f } );
    mesh_background.add_texture_coord( { 0.0f , 0.0f } );
    mesh_background.add_texture_coord( { 0.0f , 1.0f } );

    mesh_background.add_triangle_index( { 0,2,1 } );
    mesh_background.add_triangle_index( { 0,3,2 } );

    // Transformation pour le placer au bon endroit
    mesh_background.transform_apply_scale(1.28f,0.8f,1.0f);
    mesh_background.transform_apply_scale(1.35f,1.5f,1.0f);

    mesh_background.fill_empty_field_by_default();
    mesh_background_opengl.fill_vbo(mesh_background);

}

/*****************************************************************************\
 * Méthode load_start
 * Cette fonction se charge de gérer la création de l'écran d'accueil du jeu
 * Créer des mesh et gère l'affichage pour le titre du jeu
 * ainsi que les deux "cercles tournant" pour choisir le type de jeu
 * De nouveau : lecture de shader, de texture, création de mesh, gestion triangles
 * création du vbo
 * On fait la même chose pour les 3 parties
\*****************************************************************************/
void scene::load_start(){
    
    //////////////////////////title///////////////////////////////
    shader_title = read_shader("shaders/shader_cube.vert",
                               "shaders/shader_cube.frag"); PRINT_OPENGL_ERROR();
    texture_title = load_texture_file("data/score/titre.png");

    mesh_title.add_vertex( { +1.0f , +1.0f , -0.5f} );
    mesh_title.add_vertex( { +1.0f , -1.0f , -0.5f} );
    mesh_title.add_vertex( { -1.0f , -1.0f , -0.5f} );
    mesh_title.add_vertex( { -1.0f , +1.0f , -0.5f} );

    mesh_title.add_texture_coord( { 1.0f , 1.0f } );
    mesh_title.add_texture_coord( { 1.0f , 0.0f } );
    mesh_title.add_texture_coord( { 0.0f , 0.0f } );
    mesh_title.add_texture_coord( { 0.0f , 1.0f } );

    mesh_title.add_triangle_index( { 0,2,1 } );
    mesh_title.add_triangle_index( { 0,3,2 } );

    mesh_title.transform_apply_scale(1.44f,0.25f,1.0f);
    mesh_title.transform_apply_scale(0.7f,0.7f,1.0f);

    mesh_title.transform_apply_translation( {0,0.7f,0} );

    mesh_title.fill_empty_field_by_default();
    mesh_title_opengl.fill_vbo(mesh_title);

    //////////////////////////time///////////////////////////////
    shader_time = read_shader("shaders/shader_cube.vert",
                              "shaders/shader_cube.frag"); PRINT_OPENGL_ERROR();
    texture_time = load_texture_file("data/score/time.png");

    mesh_time.add_vertex( { -1.0f , -1.0f , -0.5f} );
    mesh_time.add_vertex( { -1.0f , +1.0f , -0.5f} );
    mesh_time.add_vertex( { +1.0f , +1.0f , -0.5f} );
    mesh_time.add_vertex( { +1.0f , -1.0f , -0.5f} );

    mesh_time.add_texture_coord( { 1.0f , 1.0f } );
    mesh_time.add_texture_coord( { 1.0f , 0.0f } );
    mesh_time.add_texture_coord( { 0.0f , 0.0f } );
    mesh_time.add_texture_coord( { 0.0f , 1.0f } );

    mesh_time.add_triangle_index( { 0,2,1 } );
    mesh_time.add_triangle_index( { 0,3,2 } );

    mesh_time.transform_apply_scale(0.4,0.4f,1.0f);
    //mesh_time.transform_apply_translation( {-0.65f,0.1f,0} );

    mesh_time.fill_empty_field_by_default();
    mesh_time_opengl.fill_vbo(mesh_time);


    //////////////////////////life///////////////////////////////
    shader_life = read_shader("shaders/shader_cube.vert",
                              "shaders/shader_cube.frag"); PRINT_OPENGL_ERROR();
    texture_life = load_texture_file("data/score/life.png");

    mesh_life.add_vertex( { -1.0f , -1.0f , -0.5f } );
    mesh_life.add_vertex( { -1.0f , +1.0f , -0.5f } );
    mesh_life.add_vertex( { +1.0f , +1.0f , -0.5f } );
    mesh_life.add_vertex( { +1.0f , -1.0f , -0.5f } );

    mesh_life.add_texture_coord( { 1.0f , 1.0f } );
    mesh_life.add_texture_coord( { 1.0f , 0.0f } );
    mesh_life.add_texture_coord( { 0.0f , 0.0f } );
    mesh_life.add_texture_coord( { 0.0f , 1.0f } );

    mesh_life.add_triangle_index( { 0,2,1 } );
    mesh_life.add_triangle_index( { 0,3,2 } );

    mesh_life.transform_apply_scale(0.4,0.4f,1.0f);

    mesh_life.fill_empty_field_by_default();
    mesh_life_opengl.fill_vbo(mesh_life);
}
/*****************************************************************************\
 * Méthode draw_start
 * Cette fonction gère l'affichage de l'écran d'acceuil
 * Pour les 3 parties (affichage du titre et deux "cercles tournants")
 * on fait la même chose : preparation du shader, rotation/scaling/translation optionnelles
 * affichage de la mesh
\*****************************************************************************/
void scene::draw_start(){
    mat4 rotation;

    //// Gestion de l'affichage du titre du jeu //////
    glEnable(GL_BLEND); // Gestion de la transparence
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    prepare_shader(shader_title);
    glUniformMatrix4fv(get_uni_loc(shader_title,"rotation"),1,false,rotation.pointer());  PRINT_OPENGL_ERROR();
    glUniform1f(get_uni_loc(shader_title,"scaling"),1.0);                           PRINT_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D,texture_title); PRINT_OPENGL_ERROR();
    mesh_title_opengl.draw();
    glDisable(GL_BLEND);

    rotation.set_rotation({0.0f,0.0f,1.0f},-time_advance_accueil/24.0f);

    //// Gestion de l'affichage du cerlce pour le mode de jeu time ///////
    glEnable(GL_BLEND); // Gestion de la transparence
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    prepare_shader(shader_time);
    glUniformMatrix4fv(get_uni_loc(shader_time,"rotation"),1,false,rotation.pointer());  PRINT_OPENGL_ERROR();
    glUniform2f(get_uni_loc(shader_life,"translation"),0.5f,-0.15f);           PRINT_OPENGL_ERROR();
    glUniform1f(get_uni_loc(shader_time,"scaling"),1.0);                           PRINT_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D,texture_time); PRINT_OPENGL_ERROR();
    mesh_time_opengl.draw();
    glDisable(GL_BLEND);


    //// Gestion de l'affichage du cercle pour le mode de jeu avec vies //////
    glEnable(GL_BLEND); // Gestion de la transparence
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    prepare_shader(shader_life);
    glUniformMatrix4fv(get_uni_loc(shader_life,"rotation"),1,false,rotation.pointer());  PRINT_OPENGL_ERROR();
    glUniform2f(get_uni_loc(shader_life,"translation"),-0.5f,-0.15f);           PRINT_OPENGL_ERROR();
    glUniform1f(get_uni_loc(shader_life,"scaling"),1.0);                           PRINT_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D,texture_life); PRINT_OPENGL_ERROR();
    mesh_life_opengl.draw();
    glDisable(GL_BLEND);
}

/*****************************************************************************\
 * Méthode load_fruits
 * Cette fonction se charge de gérer l'initialisation des mesh pour les fruits
 * Pour chaque case du tableau de mesh_fruits on lit le fichier objet correspondant
 * On remplit les champs nécessaires, et le vbo
\*****************************************************************************/
void scene::load_fruits()
{
	// Lecture des fichiers objets 
	// On load la mesh dans un tableau de mesh
    for(int i=0;i<nb_fruits; i++){
        switch(i){
        case 0:
            mesh_fruits[i] = load_mesh_file("data/object/Grenade/Grenade.obj");
            
            break;
        case 1:
            mesh_fruits[i] = load_mesh_file("data/object/Lemon/Lemon.obj");
            
            break;
        case 2:
            mesh_fruits[i] = load_mesh_file("data/object/Grape/Grapes.obj");
            
            break;
        case 3:
            mesh_fruits[i] = load_mesh_file("data/object/Strawberry/Strawberry.obj");
            
            break;
        case 4:
            mesh_fruits[i] = load_mesh_file("data/object/Melon/Melon.obj");
           
            break;
        case 5:
            mesh_fruits[i] = load_mesh_file("data/object/Pear/Pear.obj");
            
            break;
        case 6:
            mesh_fruits[i] = load_mesh_file("data/object/Pineapple/Pineapple.obj");
            
            break;
        case 7:
            mesh_fruits[i] = load_mesh_file("data/object/Tomato/Toemato.obj");
            
            break;
        case 8:
            mesh_fruits[i] = load_mesh_file("data/object/Cherries/Cherries.obj");
           
            break;
        case 9:
            mesh_fruits[i] = load_mesh_file("data/object/Watermelon/Watermelon.obj");
            
            break;
        case 10:
            mesh_fruits[i] = load_mesh_file("data/object/Apple/Apple1.obj");
            
            break;


        }
        // Quelques transformations nécessaires
        mesh_fruits[i].transform_apply_translation({-0.5f, -0.5f, -0.5f});
        mesh_fruits[i].transform_apply_scale(0.2f,0.2f,0.2f);
        // Remplissage des champs necessaires et du vbo
        mesh_fruits[i].fill_empty_field_by_default();
        mesh_fruits[i].fill_color(vec3(1.0,1.0,1.0));
        mesh_fruits_opengl[i].fill_vbo(mesh_fruits[i]);
    }


}

/*****************************************************************************\
 * Méthode load_coeurs
 * Cette fonction se charge de créer les mesh nécessaires pour l'affichage 
 * des coeurs représentant les vies
 * On définit aussi les positions d'affichage de ces coeurs
\*****************************************************************************/
void scene::load_coeurs()
{
	// Lecture du shader
    shader_coeurs= read_shader("shaders/shader_cube.vert",
                               "shaders/shader_cube.frag"); PRINT_OPENGL_ERROR();
    texture_coeurs=load_texture_file("data/coeur.png");

    // Création des vertex
    mesh_coeurs.add_vertex( { -1.0f , +1.0f , -0.9f } );
    mesh_coeurs.add_vertex( { -1.0f , -1.0f , -0.9f } );
    mesh_coeurs.add_vertex( { +1.0f , -1.0f , -0.9f } );
    mesh_coeurs.add_vertex( { +1.0f , +1.0f , -0.9f } );
    // Coord des textures
    mesh_coeurs.add_texture_coord( { 1.0f , 1.0f } );
    mesh_coeurs.add_texture_coord( { 1.0f , 0.0f } );
    mesh_coeurs.add_texture_coord( { 0.0f , 0.0f } );
    mesh_coeurs.add_texture_coord( { 0.0f , 1.0f } );
    // Gestion des triangles
    mesh_coeurs.add_triangle_index( { 0,2,3 } );
    mesh_coeurs.add_triangle_index( { 0,1,2 } );

    mesh_coeurs.transform_apply_scale(0.15f,0.15f,1.0f);

    mesh_coeurs.fill_empty_field_by_default();
    mesh_coeurs_opengl.fill_vbo(mesh_coeurs);
    // Positions des 3 coeurs 
    coeurs[0].pos.x()=0.9f;
    coeurs[0].pos.y()=0.8f;

    coeurs[1].pos.x()=0.7f;
    coeurs[1].pos.y()=0.8f;

    coeurs[2].pos.x()=0.5f;
    coeurs[2].pos.y()=0.8f;
}
/*****************************************************************************\
 * Méthode draw_coeurs
 * Cette fonction se charge d'afficher les coeurs représentant le nombre de vies
\*****************************************************************************/
void scene::draw_coeurs(){

    mat4 rotation;

    for(int i=0;i<nb_coeurs;i++){

        glEnable(GL_BLEND); // Transparence
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        prepare_shader(shader_coeurs);

        // Gestion rotation suivant l'axe de rotation définie à l'initialisation de l'objet
        glUniformMatrix4fv(get_uni_loc(shader_coeurs,"rotation"),1,false,rotation.pointer());  PRINT_OPENGL_ERROR();

        // Translation suivant sa nouvelle position
        glUniform2f(get_uni_loc(shader_coeurs,"translation"),coeurs[i].pos.x(),coeurs[i].pos.y());           PRINT_OPENGL_ERROR();
        glUniform1f(get_uni_loc(shader_coeurs,"scaling"),1.0f);                                             PRINT_OPENGL_ERROR();
        glBindTexture(GL_TEXTURE_2D,texture_coeurs);                                                         PRINT_OPENGL_ERROR();


        // affichage des coeurs
        mesh_coeurs_opengl.draw();
        glDisable(GL_BLEND); // Transparence
    }

}
/*****************************************************************************\
 * Méthode load_sword
 * Cette fonction se charge de créer le mesh file pour l'affichage de l'épée
 * et de gérer cette mesh (plus vbo)
\*****************************************************************************/
void scene::load_sword()
{
    mesh_sword = load_mesh_file("data/object/Axe/Axe.obj");//Master_Sword/mastersword.obj");
    mesh_sword.transform_apply_translation({0.0f, -0.65f, 0.0f});
    mesh_sword.transform_apply_scale(0.2f,0.2f,0.2f);
    mesh_sword.transform_apply_rotation({0.0f,1.0f,0.0f},-1.57);
    mesh_sword.transform_apply_rotation({0.0f,0.0f,1.0f},-1.57);

    mesh_sword.fill_empty_field_by_default();
    mesh_sword.fill_color(vec3(1.0,1.0,1.0));

    mesh_sword_opengl.fill_vbo(mesh_sword);
}
/*****************************************************************************\
 * Méthode draw_scene_accueil
 * Cette fonction se charge de gérer l'affichage des fruits dans le menu du jeu
 * Gestion de shader / Lancement du mode de jeu (Gestion après collision avec
 * les fruits et l'épée)
\*****************************************************************************/
void scene::draw_scene_acceuil()
{
    mat4 rotation;

    // Appel la fonction d'initialisation de l'accueil
    if(!initialisation_accueil){
        init_accueil();
        initialisation_accueil = true;
    }
    // Affichage du titre / "cercles tournants"
    draw_start();

    // Affichage des fruits de la page d'acceuil
    for(int i=0; i<2;i++){
        prepare_shader(shader_fruits[fruit_accueil[i].n]);
        // Gestion rotation suivant l'axe de rotation définie à l'initialisation de l'objet
        rotation.set_rotation(fruit_accueil[i].orientation,time_advance_accueil/24.0f);

        glUniformMatrix4fv(get_uni_loc(shader_fruits[fruit_accueil[i].n],"rotation"),1,false,rotation.pointer());  PRINT_OPENGL_ERROR();

        // Translation suivant sa nouvelle position
        glUniform2f(get_uni_loc(shader_fruits[fruit_accueil[i].n],"translation"),fruit_accueil[i].pos.x(),fruit_accueil[i].pos.y());           PRINT_OPENGL_ERROR();
        glUniform1f(get_uni_loc(shader_fruits[fruit_accueil[i].n],"scaling"),0.27f);                                         				   PRINT_OPENGL_ERROR();
        glBindTexture(GL_TEXTURE_2D,texture_fruits[fruit_accueil[i].n]);																	PRINT_OPENGL_ERROR();
        // affichage des fruits
        mesh_fruits_opengl[fruit_accueil[i].n].draw();

    }


    // Affichage de l'épée 
    draw_sword();


    // On appele sans cesse la fonction de gestion de collision si jamais la collision n'a pas été effectué
    if(!fruit_accueil[0].touch  && !fruit_accueil[1].touch){
        collision_fruit_accueil();

    }
    // Sinon il y a eu collision sur un des deux fruits
    else{
    	// Collision avec le fruit représentant le mode de jeu avec des vies
        if(fruit_accueil[0].visible && fruit_accueil[0].touch){
            glUniform2f(get_uni_loc(shader_fruits[fruit_accueil[0].n],"translation"),fruit_accueil[0].pos.x(),fruit_accueil[0].pos.y()-0.02f);
            fruit_accueil[0].pos.y()=fruit_accueil[0].pos.y()-0.02f;

            // Si le fruit n'est plus à l'écran alors on le rend "invisible"
            // Et on commence le mode de jeu avec des vies
            if (fruit_accueil[0].pos.y() < -1.1f){
                fruit_accueil[0].visible = false;
                begin_game_life = true;
            }
        }
        // Collision avec le fruit représentant le mode de jeu avec du temps
        else if(fruit_accueil[1].visible && fruit_accueil[1].touch){
            glUniform2f(get_uni_loc(shader_fruits[fruit_accueil[1].n],"translation"),fruit_accueil[1].pos.x(),fruit_accueil[1].pos.y()-0.02f);
            fruit_accueil[1].pos.y()=fruit_accueil[1].pos.y()-0.02f;
            // Si le fruit n'est plus à l'écran alors on le rend "invisible"
            // Et on commence le mode de jeu avec du temps
            if (fruit_accueil[1].pos.y() < -1.1f){
                fruit_accueil[1].visible = false;
                begin_game_time = true;
            }
        }


    }
    time_advance_accueil++;




}
/*****************************************************************************\
 * Méthode init_accueil
 * Initialise les deux instances de fruits servant pour le menu du jeu
 * -> Position du fruit / Orientation des fruits pour qu'ils tournent suivant 
 * l'axe vertical
\*****************************************************************************/
void scene::init_accueil(){
	// Premier fruit : fraise (type = 3)
    fruit_accueil[0].n = 3;
    fruit_accueil[0].pos.x() = -0.5f;
    fruit_accueil[0].pos.y() = -0.24f;
    fruit_accueil[0].orientation.x() = 0.0f;
    fruit_accueil[0].orientation.y() = 1.0f;
    fruit_accueil[0].orientation.z() = 0.0f;
    fruit_accueil[0].visible = true;

    // Deuxième fruit : pomme (type = 10)
    fruit_accueil[1].n = 10;
    fruit_accueil[1].pos.x() = 0.5f;
    fruit_accueil[1].pos.y() = -0.22f;
    fruit_accueil[1].orientation.x() = 0.0f;
    fruit_accueil[1].orientation.y() = 1.0f;
    fruit_accueil[1].orientation.z() = 0.0f;
    fruit_accueil[1].visible = true;

}

/*****************************************************************************\
 * Méthode draw_scene
 * Cette fonction est une des plus importante 
 * Elle se charge gérer l'affichage de la scène 3D : Elle appelle à chaque fois
 * les fonctions d'affichage de chaque élément
\*****************************************************************************/
void scene::draw_scene()
{


    // Get webcam image
    cv::Mat frame;
    capture >> frame;
    // Analyse d'image pour color tracking
    analyse_image(frame);

    // Tant qu'on a pas calibrer la manette
    // On affiche seulement le fond d'écran
    if(!choix_couleur)
    {
        draw_background();
    }

    // Si le jeu n'est pas lancé et que le calibrage n'est pas fait
    // Alors on affiche l'épée et le menu du jeu
    if(!begin_game_life && !begin_game_time && !end_game && !choix_couleur){
        //draw_sword();
        draw_scene_acceuil();
        

    }
    // Si le mode de jeu est celui avec les vies (et que ce n'est pas la fin du jeu)
    else if(begin_game_life && !end_game){
        // Affichage du score
        draw_score(type_score);
        // Affichage du meilleur score
        draw_score(type_best);

        // L'appel de cette fonction permet de rajouter un ou plusieurs fruits
        init_fruit();

        // Affichage de l'épée
        draw_sword();
        // Affichage des vies restantes
        draw_coeurs();
        // Affichage des fruits
        draw_fruit();
        // Affichage des points si un fruit / bombe est coupé
        draw_point();

        // Gestion du temps
        // Toutes les 1000*25ms on diminue le temps de rafraichissement des fruits
        if(time_advance % 1000 == 0){
            temps_raf *= 0.8;
        }
        // Incrémentation du temps (toutes les 25ms)
        time_advance++;

    }
    // Si le mode de jeu est celui avec du temps (et que ce n'est pas la fin du jeu)
    else if(begin_game_time && !end_game){
    	// Décrémentation du temps in game (compteur de 0 à 60)
        temps--;
        // Si le temps arrive à 0 alors c'est la fin du jeu
        if(temps == 0){
            end_game = true;
        }
        // Affiche le score et le meilleur score
        draw_score(type_score);
        draw_score(type_best);
        // Affiche le timer correspondant au temps restant dans la partie
        draw_timer();

		// L'appel de cette fonction permet de rajouter un ou plusieurs fruits
        init_fruit();
        // Affichage de l'épée
        draw_sword();
        // Affichage des fruits présent sur l'écran
        draw_fruit();
        // Affichage des points si il y a eu collision entre un fruit et une épée
        draw_point();
        // Gestion du temps
        // Toutes les 1000*25ms on diminue le temps de rafraichissement des fruits
        if(time_advance % 1000 == 0){
            temps_raf *= 0.8;
        }
        // Incrémentation du temps (toutes les 25ms)
        time_advance++;
    }
    // Si c'est la fin du jeu
    else if(end_game){
    	// Affichage du score final
        draw_score(type_final);
        // Si le score du joueur est supérieur au meilleur score
        // Alors on met à jour le meilleur score
        if(score > score_best){
            score_best = score;
            // On écrit dans le fichier score.txt le meilleur score
            ecriture_score();
        }
    }


}


/*****************************************************************************\
 * Méthode draw_fruit
 * Cette fonction se charge d'afficher les fruits
\*****************************************************************************/
void scene::draw_fruit(){
	// Matrice de rotation 
    mat4 rotation;
    // On parcourt tout le tableau de fruits
    // Il y aurait pu avoir des améliorations de faites sur cette partie
    // Afin d'éviter le parcours de tout le tableau
    // On a crée un tableau de 100 cases afin de ne pas se préoccuper de la gestion des fruits
    // Il n'y aura jamais 100 fruits afficher à l'écran
    for(int i = 0 ; i < NB_MAX; i++){
    	// On ne traite que les fruits qui sont à l'écran (définit de -1 à +1 sur l'axe x et y)
        if ((fruits[i].pos.x() > -1.1f && fruits[i].pos.x() < 1.1f) && (fruits[i].pos.y() > -1.1f && fruits[i].pos.y() < 1.1f ) && fruits[i].visible){
            // Préparation du shader
            prepare_shader(shader_fruits[fruits[i].n]);
            // Gestion rotation suivant l'axe de rotation définie à l'initialisation de l'objet
            rotation.set_rotation(fruits[i].orientation,time_advance/24.0f);
            glUniformMatrix4fv(get_uni_loc(shader_fruits[fruits[i].n],"rotation"),1,false,rotation.pointer());  PRINT_OPENGL_ERROR();
            // Change la position du fruit
            update_pos(fruits[i]);
            // Translation suivant sa nouvelle position
            glUniform2f(get_uni_loc(shader_fruits[fruits[i].n],"translation"),fruits[i].pos.x(),fruits[i].pos.y());           PRINT_OPENGL_ERROR();
            glUniform1f(get_uni_loc(shader_fruits[fruits[i].n],"scaling"),0.27f);                                             PRINT_OPENGL_ERROR();
            glBindTexture(GL_TEXTURE_2D,texture_fruits[fruits[i].n]);                                                         PRINT_OPENGL_ERROR();
            // Affichage des fruits
            mesh_fruits_opengl[fruits[i].n].draw();
        }
        else{
            // Si le fruit n'est plus dans l'écran alors il n'est plus visible
            fruits[i].visible = false;
        }

    }
}
/*****************************************************************************\
 * Méthode draw_background
 * Cette fonction se charge d'afficher le fond d'écran du jeu
 * Préparation du shader / Gestion texture / Affichage
\*****************************************************************************/
void scene::draw_background(){
    mat4 rotation;
    prepare_shader(shader_background);

    glUniformMatrix4fv(get_uni_loc(shader_background,"rotation"),1,false,rotation.pointer());  PRINT_OPENGL_ERROR();
    glUniform1f(get_uni_loc(shader_background,"scaling"),1.0);                           PRINT_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D,texture_background); PRINT_OPENGL_ERROR();
    mesh_background_opengl.draw();
}
/*****************************************************************************\
 * Méthode draw_sword
 * Cette fonction se charge d'afficher l'épée 
 * Cette fonction utilise les coordonnées définit pas la méthode de color tracking
 * Cette méthode suit deux points : le premier est stocké dans middle et l'autre
 * dans middle_2
 * Le premier point sert à localiser l'épée et le deuxième à gérer l'orientation 
 * de celle-ci
\*****************************************************************************/
void scene::draw_sword(){
	// Création de l'angle de rotation pour l'épée 
    float angle_rotation_sword=0;
    // Création matrice de rotation
    mat4 rotation;
    // Matrice de rotation à appliquer à l'épée pour gérer l'inclinaison
    // Elle sera intialisé correctement avec l'angle de rotation cité précédemment
    mat4 rotation_sword;
    // Si les deux points détectés par la méthode de color trackin sont sur le même axe suivant y
    // Épée orienté verticalement
    if(fabs(middle.y()-middle_2.y())<0.0001)
    {
    	// Rotation suivant si l'épée est orientée vers le bas ou vers le haut
        if(middle.x()>middle_2.x())
            angle_rotation_sword=1.57; // Rotation de + 90 degres

        else
            angle_rotation_sword=-1.57;// Rotation de -90 degres
    }
    // Sinon on oriente l'épée correctement
    else
    {
        //Angle de rotation correspond à l'arctan(différence des x entre les deux points / différence des y entre les deux points)
        angle_rotation_sword=atan((middle.x()-middle_2.x())/(middle.y()-middle_2.y()));
        // Gestion si l'épée est orienté tête vers le bas
        if(middle_2.y()>middle.y())
        {
            angle_rotation_sword+=3.14;
        }

    }
    // On définit la rotation suivant l'axe z
    angle_rotation_sword*=-1;
    rotation_sword.set_rotation({0.0f,0.0f,1.0f},angle_rotation_sword+1.57);

    // Si le jeu est lancé alors on détecte si il y a une collision
    if((begin_game_life || begin_game_time) && !end_game)
    {
        collision_fruit();
    }

    // Gestion des shaders et affichage
    prepare_shader(shader_sword);
    glUniformMatrix4fv(get_uni_loc(shader_sword,"rotation"),1,false,rotation_sword.pointer());  PRINT_OPENGL_ERROR();
    glUniform2f(get_uni_loc(shader_sword,"translation"),middle.x(),middle.y()-0.5f);           PRINT_OPENGL_ERROR();
    glUniform1f(get_uni_loc(shader_sword,"scaling"),0.5f);                           PRINT_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D,texture_sword); PRINT_OPENGL_ERROR();
    mesh_sword_opengl.draw();
}

/*****************************************************************************\
 * Méthode init_fruit
 * Cette fonction se charge d'initialiser les instances de fruits (dans le tableau)
 * avec les bons attributs de la classe Fruit
 * On parcourt petit à petit le tableau de fruit en changeant les instances
 * contenues dedans. Il n'y aura jamais 100 fruits à l'écran (tableau 100 cases)
 * Pour l'affichage des fruits on parcourt tout le tableau
 * Ici petit à petit on vient "recréer" des fruits (les rendre visibles, quel type)
 * etc.
\*****************************************************************************/
void scene::init_fruit(){
	// Si le temps est divisible par le temps de rafraichissement
	// alors on "crée" un nouveau fruit
    if(time_advance % temps_raf == 0){
    	// Calcul d'un nombre aléatoire entier entre 0 et 29
        int rand_fruits_many = (int)30*(float)rand()/RAND_MAX;
        // On crée par défaut un fruit
        int fruits_many = 1;
        // Si le nombre aléatoire est inférieur à 3 (et qu'on est pas à la fin du tableau pour éviter de déborder)
        // Cela correspond à 3 chance sur 30 = 1/10
        if(rand_fruits_many < 3 && (nb_fruits_pres <= NB_MAX-3)){
        	// Alors on va céer 2 fruits
            fruits_many = 2;
        }
        // Si le nombre aléatoire est supérieur à 27(et qu'on est pas à la fin du tableau pour éviter de déborder)
        // Cela correspond à 2 chance sur 30 = 1/15
        else if(rand_fruits_many > 27 && (nb_fruits_pres <= NB_MAX-3)){
            fruits_many = 3;
        }

        // Pour chaque fruit créé on initialise correctement chaque attribut
        for(int i=0;i<fruits_many;i++){

            nb_fruits_pres++;
            // Si on est arrivé au bout du tableau on recommence au debut
            // Permet de gérer correctement le tableau et où on est dans celui ci afin de créer le fruit au bon endroit
            // c.a.d ne pas écraser un fruit qui était visible
            if(nb_fruits_pres == NB_MAX){
                nb_fruits_pres = 1;
            }
            
            // Choisi le type de fruit au hasard parmi les nb_fruits possible (bombe comprise)
            fruits[nb_fruits_pres-1].n = rand() % nb_fruits;

            // Position du fruit suivant x est prise aléatoirement
            fruits[nb_fruits_pres-1].pos.x() = 2*(float)rand()/RAND_MAX -1;
            // On positionne le fruit en bas de l'ecran
            fruits[nb_fruits_pres-1].pos.y() = -1.09f;
            // Gestion orientation
            fruits[nb_fruits_pres-1].orientation.x() = (float)rand()/RAND_MAX;
            fruits[nb_fruits_pres-1].orientation.y() = (float)rand()/RAND_MAX;
            fruits[nb_fruits_pres-1].orientation.z() = (float)rand()/RAND_MAX;
            fruits[nb_fruits_pres-1].dt = 0.20f*(float)rand()/RAND_MAX + 0.20f;
            // Tous les 500 coups (un coup toutes les 25ms)
            // On augmente le pas d'intégration : ainsi les fruits vont plus vite
            if(time_advance % 500 == 0){
                fruits[nb_fruits_pres-1].dt += 0.05f;
            }
            // Set visible to true
            fruits[nb_fruits_pres-1].visible = true;
            // Dans un cas la vitesse du fruit sera sur la gauche (1 chance sur 2)
            // Vitesse est comprise entre 10.0 et 30.0
            if(rand() % 2){
                fruits[nb_fruits_pres-1].speed.x() = 20*(float)rand()/RAND_MAX + 10.0f;
            }
            // Sinon vers la droite (1 chance sur deux)
            else{
                fruits[nb_fruits_pres-1].speed.x() = 20*(float)rand()/RAND_MAX - 30.0f;

            }
            // Vitesse suivant y est comprise entre -70 et -110
            fruits[nb_fruits_pres-1].speed.y() = -100.0f;//-(40*(float)rand()/RAND_MAX  + 80.0f) ;
        }

    }
}

/*****************************************************************************\
 * Méthode update_pos
 * Cette fonction se charge de mettre à jour la position du fruit passé en paramètre
\*****************************************************************************/
void scene::update_pos(Fruit& fruit){

    //size of the window
    float const h=height();
    float const w=width();

    // Intégration pas à pas : update de la vitesse du fruit
    fruit.speed  = fruit.speed+fruit.dt*gravity;
    fruit.speed -= fruit.dt*damping*fruit.speed; //apply damping

    // On regarde sur quel pixel est situé le centre de gravité du fruit
    // Conversion position (x,y) vers pixel (i,j)
    vec2 pos_pix = {h/2*(fruit.pos.x()+1), w/2*(-fruit.pos.y()+1)};

    // On update la position du fruit
    pos_pix = pos_pix+fruit.dt*fruit.speed;

    // On repasse en coordonnées (x,y)
    fruit.pos = {2*pos_pix.x()/h -1,-(2*pos_pix.y()/w -1)};


    //Gestion de la collision du fruit avec les bord de l'écran
    collision_handling(fruit);
}

/*****************************************************************************\
 * Méthode collision_fruit
 * Cette fonction se charge de vérifier s'il y a collision entre un fruit et l'épée
\*****************************************************************************/
void scene::collision_fruit(){

	// On parcourt tous les fruits pour regardr la collision
    for(int i=0; i<NB_MAX; i++){
    	// On check les collisions que si le fruit est visible
        if(fruits[i].visible){
        	// Initialisation d'un float représentant la vitesse de l'épée
            float speed_sword = 0.0f;
            // On connait les 5 dernières positions de l'épée 
            // On regarde la dernière position connue de l'épée
            // On récupère le vec2 de la position correspondante
            cpe::vec2 last_pos = store_middle.front();
            //On regarde la plus vieille position de l'épée (5 coup avant)
            std::list<cpe::vec2>::iterator it=store_middle.end();
            it--;
            // On récupère le vec2 correspondant
            cpe::vec2 last_pos_prev = *it;
            //Calcul de la "vitesse" de l'épée : sqrt(différence de position suivant x au carré + différence de position suivant y au carré)
            speed_sword = sqrt((last_pos.x()-last_pos_prev.x())*(last_pos.x()-last_pos_prev.x())+(last_pos.y()-last_pos_prev.y())*(last_pos.y()-last_pos_prev.y()));
            //Si l'épée est au même endroit que le fruit courant
            // Alors ce fruit est potentiellement coupé
            if(fabs(middle.x()-fruits[i].pos.x())<epsilon_x && fabs(middle.y()-fruits[i].pos.y())<epsilon_y){
            	// Le fruit est coupé que si la vitesse de l'épée est suffisament grande
            	// Si la vitesse de l'épée est nulle alors le fruit n'est pas coupé
                if(speed_sword > 0.15f){
                    fruits[i].visible = false;
                    // Si le ffruit est coupé alors on gère le score
                    gestion_score(fruits[i]) ;
                }

            }
        }
    }
}
/*****************************************************************************\
 * Méthode collision_fruit_accueil
 * Cette fonction se charge de vérifier s'il y a collision entre un fruit et l'épée
 * dans le cas du menu du jeu
\*****************************************************************************/
void scene::collision_fruit_accueil(){
	// Deux fruits sont présent dans le menu du jeu
	// Les couper revient à lancer un mode de jeu
	// On regarde donc si un des deux fruits et coupé
    for(int i=0; i<2; i++){

        if(fruit_accueil[i].visible ){
        	// Déclaration d'un float pour la "vitesse" de l'épée
            float speed_sword = 0.0f;
            // On récupère la dernière position connue de l'épée
            cpe::vec2 last_pos = store_middle.front();
            // On récupère la plus vieille position connue de l'épée (5 coup avant)
            std::list<cpe::vec2>::iterator it=store_middle.end();
            it--;
            cpe::vec2 last_pos_prev = *it; // Récupération du vec2 de la position
            //Calcul de la "vitesse" de l'épée
            speed_sword = sqrt((last_pos.x()-last_pos_prev.x())*(last_pos.x()-last_pos_prev.x())+(last_pos.y()-last_pos_prev.y())*(last_pos.y()-last_pos_prev.y()));
            //Si l'épée est au même endroit que le fruit courant
            // Alors ce fruit est potentiellement coupé
            if(fabs(middle.x()-fruit_accueil[i].pos.x())<epsilon_x && fabs(middle.y()-fruit_accueil[i].pos.y())<epsilon_y){
            	// Le fruit est coupé que si la vitesse de l'épée est suffisament grande
            	// Si la vitesse de l'épée est nulle alors le fruit n'est pas coupé
                if(speed_sword > 0.1f){
                    fruit_accueil[i].touch = true;
                }
            }
        }
    }
}


/*****************************************************************************\
 * Méthode gestion_score
 * Cette fonction se charge de mettre à jour le score du jeu
 * On passe en paramètre l'instance de Fruit qui à été coupé afin de savoir si c'est un
 * fruit ou une bombe
\*****************************************************************************/
void scene::gestion_score(Fruit& fruit)
{
	// Si on est dans le mode de jeu avec du temps
    if(begin_game_time){
    	// Si le fruit est en fait une bombe 
    	// Alors on enlève 10 points
        if(fruit.n == 0){
        	// On crée un "-10" à faire apparaitre à l'endroit où le fruit à été coupé
            init_point(true,fruit.pos);
            // Si le score est supérieur à 10
            // Alors on retranche 10
            if(score > 10)
                score -= 10;

            else
            	// Sinon on le passe à 0
                score = 000;
        }
        // Si le fruit coupé n'est pas une grenade
        // Alors le score est incrémenté de 1
        else{
            score += 1;
            // On fait apparaitre un "+1" à l'endroit où le fruit à été coupé
            init_point(false,fruit.pos);}
    }
    // Si on est dans le mode de jeu avec des vies
    else if(begin_game_life){
    	// Si le fruit est en fait une bombe
        if(fruit.n == 0){
            //On diminue le nombre de vie
            nb_coeurs--;
            // On fait apparaitre un coeur à l'endroit où le fruit à été coupé
            init_point(true, fruit.pos);
            // Si le joueur n'a plus de vie
            // Alors c'est la fin du jeu
            if(nb_coeurs == 0){
                end_game = true;
            }
        }
        // Si le fruit coupé n'est pas une bombe
        // Alors on incrémente le score de 1
        else{
            score +=1;
            // On fait apparaitre un "+1" à l'endroit où le fruit à été coupé
            init_point(false,fruit.pos);
        }
    }

}
/*****************************************************************************\
 * Méthode load_point
 * Cette fonction se charge de créer toutes les mesh nécessaire à l'affichage
 * des points ("+1" et "-10")
 * Elle lit les shader, load les textures, et prépare les mesh
\*****************************************************************************/
void scene::load_point(){
	// Lecture du shader (même pour les deux types de points)
    shader_point= read_shader("shaders/shader_cube.vert",
                              "shaders/shader_cube.frag"); PRINT_OPENGL_ERROR();
    // Lecture des textures
    texture_point[0]=load_texture_file("data/score/-10.png");
    texture_point[1]=load_texture_file("data/score/+1.png");

    int i;
    // Pour chaque type de point on crée une mesh et on la configure correctement
    for(i=0; i<2;i++){
        mesh_point[i].add_vertex( { +1.0f , +1.0f , -0.9f } );
        mesh_point[i].add_vertex( { +1.0f , -1.0f , -0.9f } );
        mesh_point[i].add_vertex( { -1.0f , -1.0f , -0.9f } );
        mesh_point[i].add_vertex( { -1.0f , +1.0f , -0.9f } );

        mesh_point[i].add_texture_coord( { 1.0f , 1.0f } );
        mesh_point[i].add_texture_coord( { 1.0f , 0.0f } );
        mesh_point[i].add_texture_coord( { 0.0f , 0.0f } );
        mesh_point[i].add_texture_coord( { 0.0f , 1.0f } );

        mesh_point[i].add_triangle_index( { 0,2,3 } );
        mesh_point[i].add_triangle_index( { 0,1,2 } );
    }

    mesh_point[0].transform_apply_scale(0.48f,0.35f,1.0f);
    mesh_point[1].transform_apply_scale(0.35f,0.30f,1.0f);

    for(i=0; i<2;i++){
        mesh_point[i].transform_apply_scale(0.3f,0.3f,1.0f);
        mesh_point[i].fill_empty_field_by_default();
        mesh_point_opengl[i].fill_vbo(mesh_point[i]);
    }
}
/*****************************************************************************\
 * Méthode init_point
 * Cette fonction se charge d'initialiser un point ("+1" ou "-10") suivant le 
 * paramètre neg
 * Elle va correctement initialiser les attributs de cette instance de Point
 * afin de positionner ce point à l'endroit où le fruit à été coupé (param pos)
 * Dans cette fonction on gère le tableau d'instance d'objets Points 
 * Ce tableau fait une longueur NB_MAX (100 ici)
 * Chaque fois qu'on fait appel à cette fonction on "crée" une nouvelle instance
 * et on se déplace dans le tableau
\*****************************************************************************/
void scene::init_point(bool neg,vec2 pos){
	// On incrémente la position dans le tableau de points
    nb_points_pres++;
    // Si on est arriver au bout du tableau on recommence au debut
    if(nb_points_pres == NB_MAX){
        nb_points_pres = 1;
    }
    // Si le point est en fait un "-10"
    if(neg == true)
    	// Alors on dit que c'est le type 0
        points[nb_points_pres-1].type = 0;
    else
    	// Sinon c'est un "+1" : type 1
        points[nb_points_pres-1].type = 1;

    // On place le point à l'endroit où le fruit à été coupé
    // Cette position correspond au vec2 pos passé en paramètre
    points[nb_points_pres-1].pos = pos;
    // On rend le point visible
    points[nb_points_pres-1].visible = true;
    // On initialise le temps de visibilité à 20 (25ms*20=0,4s)
    // A chaque affichage (toutes les 25ms) on décrémente ce temps
    // Si le temps arrive à 0, on arrête d'afficher le points
    points[nb_points_pres-1].temps = 20;

}
/*****************************************************************************\
 * Méthode draw_point
 * Cette fonction se charge d'afficher les points ("+1" et "-10") qui sont visible
 * Elle va parcourir tous le tableau d'instance de points et regarder lesquels
 * on doit afficher
\*****************************************************************************/
void scene::draw_point(){

    mat4 rotation;
    // On parcourt tout le tableau de points
    for(int i=0; i<NB_MAX;i++){
    	// On ne gère que le cas de ceux qui sont visibles
    	// Et qui on leur temps > 0 
        if(points[i].visible && points[i].temps > 0 ){
        	// On décrémente leur temps de visibilité
            points[i].temps--;
            // Transparence
            glEnable(GL_BLEND); 
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            // Préparation du shader
            prepare_shader(shader_point);
            // Gestion rotation suivant l'axe de rotation définie à l'initialisation de l'objet
            glUniformMatrix4fv(get_uni_loc(shader_point,"rotation"),1,false,rotation.pointer());  PRINT_OPENGL_ERROR();

            // Translation suivant sa nouvelle position
            // Qui correspond à la position initialisée précedemment avec l'endroit où le fruit à été coupé
            glUniform2f(get_uni_loc(shader_point,"translation"),points[i].pos.x(),points[i].pos.y());           PRINT_OPENGL_ERROR();
            glUniform1f(get_uni_loc(shader_point,"scaling"),1.0f);                                             PRINT_OPENGL_ERROR();
            // Si le point est de type 1 : "+1"
            // Alors on utilise la bonne texture du tableau de texture_point
            if (points[i].type==1){
                glBindTexture(GL_TEXTURE_2D,texture_point[points[i].type]);                                                        PRINT_OPENGL_ERROR();
                mesh_point_opengl[points[i].type].draw();// Affichage
            }
            // Si le point est de type 0 : "-10" et qu'on est dans le mode de jeu avec du temps
            // Alors on on affiche un point avec la texture correspondante à "-10"
            else if (points[i].type==0 && begin_game_time){
                glBindTexture(GL_TEXTURE_2D,texture_point[points[i].type]);                                                        PRINT_OPENGL_ERROR();
                mesh_point_opengl[points[i].type].draw();// Affichage
            }
            // Si le point est de type 0 : "-10" et qu'on est dans le mode de jeu avec des vies
            // Alors on on affiche un coeur pour indiquer que le joueur vient de perdre une vie
            else if(points[i].type==0 && begin_game_life){
                glBindTexture(GL_TEXTURE_2D,texture_coeurs);                                                         PRINT_OPENGL_ERROR();
                mesh_coeurs_opengl.draw(); // Affichage 
            }


            glDisable(GL_BLEND); // Transparence
        }
    }

}
/*****************************************************************************\
 * Méthode collision_handling
 * Cette fonction gère la collision entre les fruits / bombe avec les bords 
 * gauche et droit de l'écran de jeu
\*****************************************************************************/
void scene::collision_handling(Fruit& fruit)
{

    //size of the window
    float const h=height();
    float const w=width();

    //radius of the sphere
    // Ce rayon à été défini expérimentalement suivant la taille approximative des fruits
    float const r=20.0f;

    //special condition in cases of collision
    bool collision=false;
    //bool collision_wall=false;
    vec2 p = {h/2*(fruit.pos.x()+1), w/2*(-fruit.pos.y()+1)};
    //std::cout << p << std::endl;

    //collision with the left wall
    if(p.x()-r<0)
    {
        p.x()=r;
        fruit.pos = {2*p.x()/h -1,-(2*p.y()/w -1)};
        fruit.speed.x() *= -1;
        collision=true;
    }
    //collision with the right wall
    if(p.x()+r>h)
    {
        p.x()=h-r;
        fruit.pos = {2*p.x()/h -1,-(2*p.y()/w -1)};
        fruit.speed.x() *= -1;
        collision=true;
    }


    //decrease speed with respect to the bouncing coefficient
    if(collision)
        fruit.speed *= bounce_coeff;

}

/*****************************************************************************\
 * Méthode start_webcam
 * Cette fonction se charge de démarrer la webcam
\*****************************************************************************/
void scene::start_webcam()
{
    capture=cv::VideoCapture(0);
    if(!capture.isOpened())
    {
        std::cerr<<"Failed to open Camera"<<std::endl;
        exit(1);
    }
}
/*****************************************************************************\
 * Méthode load_common_data
 * Cette fonction se charge de lire les shaders pour la webcam, l'épée etc
 * Elle se charge aussi de charger les textures pour tous les fruits, l'épée
\*****************************************************************************/
void scene::load_common_data()
{
    // Épée : Load texture
    texture_sword = load_texture_file("data/object/Axe/Texture/Axe_Texture.png");
    // Lecture du shader pour les mesh
    shader_mesh = read_shader("shaders/shader_mesh.vert",
                              "shaders/shader_mesh.frag"); PRINT_OPENGL_ERROR();

    shader_cube = read_shader("shaders/shader_cube.vert",
                              "shaders/shader_cube.frag"); PRINT_OPENGL_ERROR();
    // Lecture du shader pour l'épée
    shader_sword = read_shader("shaders/shader_cube.vert",
                               "shaders/shader_cube.frag"); PRINT_OPENGL_ERROR();

    // Lecture du shader pour la webcam
    shader_webcam = read_shader("shaders/shader_webcam.vert",
                                "shaders/shader_webcam.frag"); PRINT_OPENGL_ERROR();
    // Pour tous les fruits on va lire le shader et charger la texture correspondante au fruti
    // Ces textures sont stockées dans un tableau de texture
    for(int i=0; i<nb_fruits; i++){
    	// Même shader pour tous les fruits
        shader_fruits[i] = read_shader("shaders/shader_cube.vert",
                                       "shaders/shader_cube.frag"); PRINT_OPENGL_ERROR();
        // Chargement des différentes textures suivant le type de fruit
        switch(i){
        case 0:
            texture_fruits[i]=load_texture_file("data/object/Grenade/Textures/Metal.png");
            break;
        case 1:
            texture_fruits[i]=load_texture_file("data/object/Lemon/Texture/Lemon_Texture.png");
            break;
        case 2:
            texture_fruits[i]=load_texture_file("data/object/Grape/Textures/Grapes_Texture.png");
            break;
        case 3:
            texture_fruits[i]=load_texture_file("data/object/Strawberry/Texture/Strawberry_Texture.png");
            break;
        case 4:
            texture_fruits[i]=load_texture_file("data/object/Melon/Texture/Melon_Texture.png");
            break;
        case 5:
            texture_fruits[i]=load_texture_file("data/object/Pear/Texture/Pear_Texture.png");
            break;
        case 6:
            texture_fruits[i]=load_texture_file("data/object/Pineapple/Texture/Pineapple_Texture.png");
            break;
        case 7:
            texture_fruits[i]=load_texture_file("data/object/Tomato/Texture/Toemato_Texture.png");
            break;
        case 8:
            texture_fruits[i]=load_texture_file("data/object/Cherries/Texture/Cherries_Texture.png");
            break;
        case 9:
            texture_fruits[i]=load_texture_file("data/object/Watermelon/Texture/Watermelon_Texture.png");
            break;
        case 10:
            texture_fruits[i]=load_texture_file("data/object/Apple/Texture/Apple_Texture.png");
            break;



        }


    }

}


/*****************************************************************************\
 * Méthode prepare_shader
 * Cette fonction se charge de préparer les shader 
 * et d'envoyer les données au GPU
\*****************************************************************************/
void scene::prepare_shader(GLuint const shader_id)
{
    //Setup uniform parameters
    glUseProgram(shader_id);                                                                           PRINT_OPENGL_ERROR();

    //Get cameras parameters (modelview,projection,normal).
    camera_matrices const& cam=pwidget->camera();

    //Set Uniform data to GPU
    glUniformMatrix4fv(get_uni_loc(shader_id,"camera_modelview"),1,false,cam.modelview.pointer());     PRINT_OPENGL_ERROR();
    glUniformMatrix4fv(get_uni_loc(shader_id,"camera_projection"),1,false,cam.projection.pointer());   PRINT_OPENGL_ERROR();
    glUniformMatrix4fv(get_uni_loc(shader_id,"normal_matrix"),1,false,cam.normal.pointer());           PRINT_OPENGL_ERROR();
}


/*****************************************************************************\
 * Méthode generate_texture_webcam
 * Cette fonction se charge de créer une texture à partir du flux vidéo de la
 * webcam afin qu'on puisse afficher ce que la webcam récupère
\*****************************************************************************/
GLuint scene::generate_texture_webcam(cv::Mat const& im)
{
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);  PRINT_OPENGL_ERROR();

    GLenum in_color = GL_BGR;
    if (im.channels() == 1)
        in_color = GL_LUMINANCE;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); PRINT_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); PRINT_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); PRINT_OPENGL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); PRINT_OPENGL_ERROR();

    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,im.cols,im.rows,0,in_color,GL_UNSIGNED_BYTE,im.ptr()); PRINT_OPENGL_ERROR();

    return texture_id;
}
/*****************************************************************************\
 * Constructeur par défaut de la classe scene
\*****************************************************************************/
scene::scene()
    :shader_mesh(0),shader_cube(0),time_advance(0),time_advance_accueil(0)
{}

/*****************************************************************************\
 * Méthode load_texture_file
 * Cette fonction se sert de la méthode load_texture_file de la classe pwidget
 * pour lire le fichier dont le nom est passé en paramètre
\*****************************************************************************/
GLuint scene::load_texture_file(std::string const& filename)
{
    return pwidget->load_texture_file(filename);
}

void scene::set_widget(myWidgetGL* widget_param)
{
    pwidget=widget_param;
}
/*****************************************************************************\
 * Méthode load_best
 * Cette fonction s'occupe de charger toutes les textures relatives au meilleur
 * score. Elle charge toutes les textures de tous les chiffres pour 
 * l'affichage du meilleur score
 * Elle lit aussi le shader nécessaire et créer une mesh pour l'affichage
 * du mot "best" (+ lecture de la texture correspondante)
\*****************************************************************************/
void scene::load_best(){

	// Chargement de toutes les textures pour l'affichage du meilleur score
    texture_chiffres_best.push_back(load_texture_file("data/score/0_best.png"));
    texture_chiffres_best.push_back(load_texture_file("data/score/1_best.png"));
    texture_chiffres_best.push_back(load_texture_file("data/score/2_best.png"));
    texture_chiffres_best.push_back(load_texture_file("data/score/3_best.png"));
    texture_chiffres_best.push_back(load_texture_file("data/score/4_best.png"));
    texture_chiffres_best.push_back(load_texture_file("data/score/5_best.png"));
    texture_chiffres_best.push_back(load_texture_file("data/score/6_best.png"));
    texture_chiffres_best.push_back(load_texture_file("data/score/7_best.png"));
    texture_chiffres_best.push_back(load_texture_file("data/score/8_best.png"));
    texture_chiffres_best.push_back(load_texture_file("data/score/9_best.png"));

    // Lecture shader pour l'affichage du mot "best"
    shader_best= read_shader("shaders/shader_cube.vert",
                             "shaders/shader_cube.frag"); PRINT_OPENGL_ERROR();
    // Texture correspondate au mot "best"
    texture_best=load_texture_file("data/score/best.png");

    // On crée une mesh pour y placer la texture
    mesh_best.add_vertex( { +1.0f , +1.0f , -0.99f } );
    mesh_best.add_vertex( { +1.0f , -1.0f , -0.99f } );
    mesh_best.add_vertex( { -1.0f , -1.0f , -0.99f } );
    mesh_best.add_vertex( { -1.0f , +1.0f , -0.99f } );

    mesh_best.add_texture_coord( { 1.0f , 1.0f } );
    mesh_best.add_texture_coord( { 1.0f , 0.0f } );
    mesh_best.add_texture_coord( { 0.0f , 0.0f } );
    mesh_best.add_texture_coord( { 0.0f , 1.0f } );

    mesh_best.add_triangle_index( { 0,2,3 } );
    mesh_best.add_triangle_index( { 0,1,2 } );

    mesh_best.transform_apply_scale(0.62f,0.18f,1.0f);

    mesh_best.fill_empty_field_by_default();
    mesh_best_opengl.fill_vbo(mesh_best);
}

/*****************************************************************************\
 * Méthode draw_best
 * Cette fonction se charge d'afficher le meilleur score
\*****************************************************************************/
void scene::draw_best(){

    mat4 rotation;
    // Transparence
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Préparation du shader
    prepare_shader(shader_best);

    // Gestion rotation suivant l'axe de rotation définie à l'initialisation de l'objet
    glUniformMatrix4fv(get_uni_loc(shader_best,"rotation"),1,false,rotation.pointer());  PRINT_OPENGL_ERROR();

    // Translation suivant la position où on veut afficher le meilleur score (en haut à gauche)
    glUniform1f(get_uni_loc(shader_best,"scaling"),0.2f);
    glUniform2f(get_uni_loc(shader_best,"translation"),-0.85f,0.6f);           PRINT_OPENGL_ERROR();
    PRINT_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D,texture_best);

    // Affichage
    mesh_best_opengl.draw();
    glDisable(GL_BLEND); // Transparence
}

/*****************************************************************************\
 * Méthode load_final
 * Cette fonction se charge de créer une mesh texturé pour l'affichage du 
 * titre final du jeu (écran final) qui correspond au score final du joueur
\*****************************************************************************/
void scene::load_final(){

	// Lecture du shader
    shader_final= read_shader("shaders/shader_cube.vert",
                              "shaders/shader_cube.frag"); PRINT_OPENGL_ERROR();
    // Chargement de la texture du titre
    texture_final=load_texture_file("data/score/titrefin.png");
    // Création d'une mesh où l'on va placer la texture
    mesh_final.add_vertex( { +1.0f , +1.0f , -0.99f } );
    mesh_final.add_vertex( { +1.0f , -1.0f , -0.99f } );
    mesh_final.add_vertex( { -1.0f , -1.0f , -0.99f } );
    mesh_final.add_vertex( { -1.0f , +1.0f , -0.99f } );

    mesh_final.add_texture_coord( { 1.0f , 1.0f } );
    mesh_final.add_texture_coord( { 1.0f , 0.0f } );
    mesh_final.add_texture_coord( { 0.0f , 0.0f } );
    mesh_final.add_texture_coord( { 0.0f , 1.0f } );

    mesh_final.add_triangle_index( { 0,2,3 } );
    mesh_final.add_triangle_index( { 0,1,2 } );

    mesh_final.transform_apply_scale(1.13f,0.22f,1.0f);

    mesh_final.fill_empty_field_by_default();
    mesh_final_opengl.fill_vbo(mesh_final);
}

/*****************************************************************************\
 * Méthode draw_final
 * Cette fonction se charge d'afficher le score final du joueur
\*****************************************************************************/
void scene::draw_final(){

    mat4 rotation;
    // Transparence
    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Préparation du shader
    prepare_shader(shader_final);

    // Gestion rotation suivant l'axe de rotation définie à l'initialisation de l'objet
    glUniformMatrix4fv(get_uni_loc(shader_final,"rotation"),1,false,rotation.pointer());  PRINT_OPENGL_ERROR();

    // Translation suivant la position où on veut afficher le titre (au centre de l'écran)
    glUniform1f(get_uni_loc(shader_final,"scaling"),1.0f);
    glUniform2f(get_uni_loc(shader_final,"translation"),0.0f,0.4f);           PRINT_OPENGL_ERROR();
    PRINT_OPENGL_ERROR();
    glBindTexture(GL_TEXTURE_2D,texture_final);

    // Affichage
    mesh_final_opengl.draw();
    glDisable(GL_BLEND); // Transparence
}

/*****************************************************************************\
 * Méthode load_chiffres
 * Cette fonction s'occupe de charger tous les éléments relatifs à l'affichage
 * du score du joueur : texture pour les chiffres, création de mesh pour
 * affichage de chaque chiffres (3 chiffres à afficher)
 * Cela correspond aux chiffres normaux pour le score courant du joueur
\*****************************************************************************/
void scene::load_chiffres(){

    int i;
    // Pour chaque chiffres de 0 à 9
    for(i=0;i<10;i++){
    	// Lecture du shader
        shader_chiffres[i]= read_shader("shaders/shader_cube.vert",
                                        "shaders/shader_cube.frag"); PRINT_OPENGL_ERROR();
        // Création d'une mesh
        mesh_chiffres[i].add_vertex( { +1.0f , +1.0f , -0.99f } );
        mesh_chiffres[i].add_vertex( { +1.0f , -1.0f , -0.99f } );
        mesh_chiffres[i].add_vertex( { -1.0f , -1.0f , -0.99f } );
        mesh_chiffres[i].add_vertex( { -1.0f , +1.0f , -0.99f } );

        mesh_chiffres[i].add_texture_coord( { 1.0f , 1.0f } );
        mesh_chiffres[i].add_texture_coord( { 1.0f , 0.0f } );
        mesh_chiffres[i].add_texture_coord( { 0.0f , 0.0f } );
        mesh_chiffres[i].add_texture_coord( { 0.0f , 1.0f } );

        mesh_chiffres[i].add_triangle_index( { 0,2,3 } );
        mesh_chiffres[i].add_triangle_index( { 0,1,2 } );

        mesh_chiffres[i].transform_apply_scale(0.075f,0.16f,1.0f);

        mesh_chiffres[i].fill_empty_field_by_default();
        mesh_chiffres_opengl[i].fill_vbo(mesh_chiffres[i]);
    }

    // Chargement des textures chiffre de 0 à 9
    texture_chiffres.push_back(load_texture_file("data/score/0.png"));
    texture_chiffres.push_back(load_texture_file("data/score/1.png"));
    texture_chiffres.push_back(load_texture_file("data/score/2.png"));
    texture_chiffres.push_back(load_texture_file("data/score/3.png"));
    texture_chiffres.push_back(load_texture_file("data/score/4.png"));
    texture_chiffres.push_back(load_texture_file("data/score/5.png"));
    texture_chiffres.push_back(load_texture_file("data/score/6.png"));
    texture_chiffres.push_back(load_texture_file("data/score/7.png"));
    texture_chiffres.push_back(load_texture_file("data/score/8.png"));
    texture_chiffres.push_back(load_texture_file("data/score/9.png"));

    // Emplacement du score 
    // On place les 3 chiffres les uns à coté des autres en haut à gauche de l'écran
    chiffres_score.push_back({-0.9f,0.8f});
    chiffres_score.push_back({-0.8f,0.8f});
    chiffres_score.push_back({-0.7f,0.8f});

    // Emplacement du temps
    // On place le timer en haut à droite de l'écran 
    // Timer composé de deux chiffres côte à côte
    chiffres_temps.push_back({0.8f,0.8f});
    chiffres_temps.push_back({0.9f,0.8f});

    // Emplacement du meilleur score
    // Ce meilleur score est afficher en dessous du score courant du joueur
    // Il est composé de 3 chiffres côte à côte
    chiffres_best_score.push_back({-0.7f,0.605f});
    chiffres_best_score.push_back({-0.65f,0.605f});
    chiffres_best_score.push_back({-0.6f,0.605f});

    // Emplacement du score final
    // Ce score sera afficher au centre de l'écran dans l'écran de fin de jeu
    // Il est composé de 3 chiffres
    chiffres_score_final.push_back({-0.1f,0.0f});
    chiffres_score_final.push_back({-0.0f,0.0f});
    chiffres_score_final.push_back({ 0.1f,0.0f});
}
/*****************************************************************************\
 * Méthode draw_score
 * Cette fonction se charge d'afficher le score du joueur
 * Le paramètre enum Type t correspond au type de score à afficher : meilleur
 * score, score normal, score final
 * On utilise le tableau unites[i] qui contient {100,10,1,10,1} 
 * ce qui permet de gérer les chiffres des centaines puis des dizaine puis des unites
 * en divisant notre score pour unites[i] et en prenant la partie entière du
 * résultat
\*****************************************************************************/
void scene::draw_score(enum Type t){
    mat4 rotation;
    // Entier correspondant au score
    int s=000;
    // Float pour gérer la taille de l'affichage
    float scale=1.0f;

    // Vector de vec2 pour l'emplacement des chiffres
    std::vector<vec2> chiffres(3);
    // Vector de Gluint pour les textures des chiffres de 0 à 9
    std::vector<GLuint> texture(10);

    // Si c'est le score classique qu'on veut afficher
    if(t==type_score){
    	// On récupère le score
        s=score;
        // Ainsi que l'emplacement des chiffres qui le composent
        chiffres=chiffres_score;
        // Ainsi que les textures correspondantes
        texture=texture_chiffres;
    }
    // Si on veut afficher le meilleur score
    else if(t==type_best){
    	// On récupère le meilleur score
        s=score_best;
        // Ainsi que l'emplacement des chiffres qui le composent
        chiffres=chiffres_best_score;
        // On récupère aussi les textures
        texture=texture_chiffres_best;
        // On affiche le score en plus petit
        scale=0.3f;
        // On appelle la fonction d'affichage
        draw_best();
    }
    // Si on veut afficher le score final
    else if(t==type_final){
    	// On récupère le score du joueur 
        s=score;
        // On récupère aussi l'emplacement des chiffres
        chiffres=chiffres_score_final;
        // Ainsi que les textures
        texture=texture_chiffres;
        // On appelle la fonction d'affichage
        draw_final();
    }
    // Sinon c'est un mauvais type de score
    else
        std::cout<<"mauvais type de score"<<std::endl;
    // Pour les 3 chiffres qui compose le score (max 999)
    for(int i=0;i<3;i++){
    	// Transparence
        glEnable(GL_BLEND); 
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // Préparation du shader
       
        prepare_shader(shader_chiffres[(s/unites[i])%10]);

        // Gestion rotation suivant l'axe de rotation définie à l'initialisation de l'objet
        glUniformMatrix4fv(get_uni_loc(shader_chiffres[(s/unites[i])%10],"rotation"),1,false,rotation.pointer());  PRINT_OPENGL_ERROR();

        // Translation suivant la position que nous avons récupérée précedemment
        glUniform2f(get_uni_loc(shader_chiffres[(s/unites[i])%10],"translation"),chiffres[i].x(),chiffres[i].y());           PRINT_OPENGL_ERROR();
        glUniform1f(get_uni_loc(shader_chiffres[(s/unites[i])%10],"scaling"),scale);                                             PRINT_OPENGL_ERROR();
        glBindTexture(GL_TEXTURE_2D,texture[(s/unites[i])%10]);

        // Affichage
        mesh_chiffres_opengl[(s/100)%10].draw();
        glDisable(GL_BLEND); // Transparence
    }
}
/*****************************************************************************\
 * Méthode draw_timer
 * Cette fonction se charge d'afficher le timer
 * Ce timer part de 60 jusqu'à 0 : il a donc 2 chiffres
 * On utilise le tableau unites[i] qui contient {100,10,1,10,1} 
 * ce qui permet de gérer les chiffres des centaines puis des dizaine puis des unites
 * en divisant notre score pour unites[i] et en prenant la partie entière du
 * résultat
 * On divise notre temps par 40 car notre temps va de 2400 à 0 car la clock
 * est de 25 ms et 25ms*40 = 1s
 * On veut afficher le temps de 60s a 0s par pas de 1s d'où cette division par 40
\*****************************************************************************/
void scene::draw_timer(){
    mat4 rotation;
    // Pour les deux chiffres du timer
    for(int i=0;i<2;i++){
    	// Transparence
        glEnable(GL_BLEND); 
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // Préparation du shader
        prepare_shader(shader_chiffres[(temps/(4*unites[i+1]))%10]);

        // Gestion rotation suivant l'axe de rotation définie à l'initialisation de l'objet
        glUniformMatrix4fv(get_uni_loc(shader_chiffres[(temps/(40*unites[i+1]))%10],"rotation"),1,false,rotation.pointer());  PRINT_OPENGL_ERROR();

        // Translation suivant la position où le veut placer le timer
        glUniform2f(get_uni_loc(shader_chiffres[(temps/(40*unites[i+1]))%10],"translation"),chiffres_temps[i].x(),chiffres_temps[i].y());           PRINT_OPENGL_ERROR();
        glUniform1f(get_uni_loc(shader_chiffres[(temps/(40*unites[i+1]))%10],"scaling"),1.0f);                                             PRINT_OPENGL_ERROR();
        glBindTexture(GL_TEXTURE_2D,texture_chiffres[(temps/(40*unites[i+1]))%10]);                                                         PRINT_OPENGL_ERROR();


        // Affichage
        mesh_chiffres_opengl[(temps/(40*unites[i+1]))%10].draw();
        glDisable(GL_BLEND); // Transparence
    }


}
/*****************************************************************************\
 * Méthode lecture_score
 * Cette fonction de charge de lire le meilleur score stocké dans le fichier
 * score.txt
 * Elle lit ce score et le sauvegarde dans la variable score_best
\*****************************************************************************/
void scene::lecture_score(){
    std::ifstream ifs;
    ifs.open("score.txt",std::ifstream::in);

    if(ifs.good()!=true)
        std::cout<<"score.txt n'a pas pu être ouvert"<<std::endl;
    else
    {
        while(ifs.good()){
            int buffer;
            ifs >> buffer;
            score_best = buffer;
            if(ifs.good())
                std::cout<<buffer<<std::endl;
        }
    }
    ifs.close();
}
/*****************************************************************************\
 * Méthode ecriture_score
 * Cette fonction de charge d'écrire le meilleur score dans le fichier
 * score.txt
\*****************************************************************************/
void scene::ecriture_score(){
    std::ofstream ofs;
    ofs.open("score.txt",std::ofstream::out);
    if(ofs.good()){
        ofs<<score_best;
    }
    else
        std::cout<<"Cannot open score.txt"<<std::endl;
    ofs.close();
}

/*****************************************************************************\
 * Méthode binarisation
 * Cette fonction crée un masque binaire, dont les seuls pixels blancs correspondent
 * aux pixels de l'image dont la couleur correspond au critère de sélection
 * On recueille les positions de ces pixels dans deux tableaux (un pour les abscisses et un pour les ordonnées)
 * et nous en faisons la moyenne
\*****************************************************************************/
cpe::vec2 scene::binarisation(cv::Mat image, int *nbPixels, int Color_choix){//IplImage* image, int *nbPixels, int Color_choix) {

    int x, y;
    cv::Scalar pixel;
    cv::Mat hsv, mask(480,640,CV_8UC1);
    int sommeX = 0, sommeY = 0;
    *nbPixels = 0;


    // Create the hsv image
    hsv = image.clone();
    cv::cvtColor(image, hsv, CV_BGR2HSV);


    // Create kernels for the morphological operation
    cv::Mat kernel = cv::getStructuringElement( CV_SHAPE_ELLIPSE, cv::Size( 5, 5), cv::Point( 1, 1 ) );


    if(Color_choix)
    {
        // We create the mask for the right color
        cv::inRange(hsv,cv::Scalar(h_r - tolerance_r -1, s_r - tolerance_r, 0), cv::Scalar(h_r + tolerance_r -1, s_r + tolerance_r, 255), mask);
    }
    else
    {
        // We create the mask for the left color
        cv::inRange(hsv, cv::Scalar(h_l - tolerance_l -1, s_l - tolerance_l, 0), cv::Scalar(h_l + tolerance_l -1, s_l + tolerance_l, 255), mask);
    }

    if(choix_couleur && Color_choix == Couleur_Side)
    {
        cv::imshow("Masque Calibrage",mask); // We show the last chosen mask
    }

    // Morphological opening (inverse because we have white pixels on black background)
    cv::dilate(mask, mask, kernel);
    cv::erode(mask, mask, kernel);

    // We go through the mask to look for the tracked object and get its gravity center
    for(x = 0; x < mask.cols; x++) {
        for(y = 0; y < mask.rows; y++) {

            // If its a tracked pixel, count it to the center of gravity's calcul
            if(((uchar *)(mask.data + y*mask.cols))[x] == 255) {
                sommeX += x;
                sommeY += y;
                (*nbPixels)++;
            }
        }
    }

    // We release the memory of the mask
     mask.release();

    // If there is no pixel, we return a center outside the image, else we return the center of gravity
    if(*nbPixels > 0)
        return cpe::vec2((int)(sommeX / (*nbPixels)), (int)(sommeY / (*nbPixels)));
    else
        return cpe::vec2(-1, -1);
}


/*****************************************************************************\
 * Méthode CalculObjectPos
 * Cette fonction calcule la prochaine position du centre par rapport à la posiiton actuelle.
 * Elle permet de gérer les cas particulier tel que deux positions trop proches il n'y a pas
 * de modification, si on est hors du rayon de la camera on met la position par défaut à (-5,-5)
 * C'est également elle qui se charge d'afficher les fenêtres de calibrages, et ajoute des cercles rouges
 * a la position des centres
\*****************************************************************************/

cpe::vec2 scene::CalculObjectPos(cv::Mat image, cpe::vec2 objectNextPos, int nbPixels,cpe::vec2 objectPos) {

    int objectNextStepX, objectNextStepY;

    // Calculate circle next position (if there is enough pixels)
    if (nbPixels > NB_PIXEL) {

        // Reset position if no pixel were found
        if (objectPos.x() == -1 || objectPos.y() == -1) {
            objectPos.x() = objectNextPos.x();
            objectPos.y() = objectNextPos.y();
        }

        // Move step by step the object position to the desired position
        if (abs(objectPos.x() - objectNextPos.x()) > STEP_MIN) {
            objectNextStepX = max(STEP_MIN, min(STEP_MAX, abs(objectPos.x() - objectNextPos.x()) / 2));
            objectPos.x() += (-1) * sign(objectPos.x() - objectNextPos.x()) * objectNextStepX;
        }
        if (abs(objectPos.y() - objectNextPos.y()) > STEP_MIN) {
            objectNextStepY = max(STEP_MIN, min(STEP_MAX, abs(objectPos.y() - objectNextPos.y()) / 2));
            objectPos.y() += (-1) * sign(objectPos.y() - objectNextPos.y()) * objectNextStepY;
        }

        // -1 = object isn't within the camera range
    } else {

        objectPos.x() = -5;
        objectPos.y() = -5;
    }

    if(choix_couleur) // If we are calibrating the color
    {
        // Draw an object (circle) centered on the calculated center of gravity
        if (nbPixels > NB_PIXEL)
            cv::circle(image, cv::Point(objectPos.x(),objectPos.y()), 15, CV_RGB(255, 0, 0), -1);
        cv::imshow("Calibrage Couleur", image);
        cv::createTrackbar("Tolerance Right","Calibrage Couleur",&tolerance_r,100,0,0);
        cv::createTrackbar("Tolerance Left","Calibrage Couleur",&tolerance_l,100,0,0);
    }
    return objectPos;

}
/*****************************************************************************\
 * Méthode getObjectColor
 * Cette fonction est appelé à chaque clic de souris. Elle nous permet de recueillir
 * les coordonnées hsv du pixel sélectionné dans l'image.
 * Elle met ensuite à jour les variables permettant la sélection des pixels en fonction de leur couleur
\*****************************************************************************/

void scene::getObjectColor(int event, int x, int y, int flags, void *param=NULL) {

    cv::Mat hsv;
    if(event == CV_EVENT_LBUTTONUP) {
        Couleur_Side=0;
        // Get the hsv image
        hsv = image.clone();
        cv::cvtColor(image, hsv, CV_BGR2HSV);

        // Get the selected pixel
        cv::Vec3b pixel = hsv.at<cv::Vec3b>(y,x);

        // Change the value of the tracked color with the color of the selected pixel
        h_l = pixel.val[0];
        s_l = pixel.val[1];
        v_l = pixel.val[2];

    }
    if(event == CV_EVENT_RBUTTONUP) {
        Couleur_Side=1;
        // Get the hsv image
        hsv = image.clone();
        cv::cvtColor(image, hsv, CV_BGR2HSV);


        // Get the selected pixel
        cv::Vec3b pixel = hsv.at<cv::Vec3b>(y,x);

        // Change the value of the tracked color with the color of the selected pixel
        cv::circle(image, cv::Point(x,y), 5, CV_RGB(0, 255, 0), -1);

        h_r = pixel.val[0];
        s_r = pixel.val[1];
        v_r = pixel.val[2];

    }

    // Release the memory of the hsv image
    hsv.release();

}

/*****************************************************************************\
 * Méthode analyse_image
 * Cette fonction reçoit une image en paramètre et à l'aide des fonctions précédentes
 * détermine les positions des milieux des couleurs sélectionnées. On les place dans un tableau
 * et nous en faisons la moyenne.
\*****************************************************************************/
void scene::analyse_image(cv::Mat img){

    image=img.clone();


    if(choix_couleur) //Calibrating Phase
    {
        cv::namedWindow("Masque Calibrage", CV_WINDOW_AUTOSIZE);
        cv::namedWindow("Calibrage Couleur", CV_WINDOW_AUTOSIZE);

        // Mouse event to select the tracked color on the original image
        cv::setMouseCallback("Calibrage Couleur", getObjectColor, &image);

    }

    // We calculate the right color position
    objectNextPos_r = binarisation(image, &nbPixels,COLOR_RIGHT);
    vec2 centre =CalculObjectPos(image, objectNextPos_r, nbPixels,objectPos_r);

    // We calculate the left color position
    objectNextPos_l = binarisation(image, &nbPixels,COLOR_LEFT);
    vec2 centre_2= CalculObjectPos(image, objectNextPos_l, nbPixels,objectPos_l);


    //We change the value of middle and middle_2
    if((centre.x()==-5 && centre.y()==-5) || (centre_2.x()==-5 && centre_2.y()==-5))
    {
        middle={-5,-5};
        middle_2={-5,-5};
    }
    else
    {   //centre is in pixel we have to convert it in coordinates
        centre.x()=-(2*centre.x()/width()-1);
        centre.y()=-(2*centre.y()/height()-1);

        //We strore the 5 last values of center
        store_middle.push_back(centre);
        if(store_middle.size()>5)
            store_middle.pop_front();

        for(vec2 p:store_middle)
        {
            middle+=p;
        }
        middle/=store_middle.size(); //The average gives us the value of middle

        //centre_2 is in pixel we have to convert it in coordinates
        centre_2.x()=-(2*centre_2.x()/width()-1);
        centre_2.y()=-(2*centre_2.y()/height()-1);

        //We strore the 5 last values of center_2
        store_middle_2.push_back(centre_2);
        if(store_middle_2.size()>5)
            store_middle_2.pop_front();

        for(vec2 q:store_middle_2)
        {
            middle_2+=q;
        }
        middle_2/=store_middle_2.size(); //The average gives us the value of middle_2
    }


    // We wait 10 ms
    key = cvWaitKey(10);
    if(key=='q' || key=='Q') //If we press Q or q, it finishes the calibrating phase
    {
        // We destroy all the windows created
        choix_couleur=false;
        cv::destroyAllWindows();
    }


}

