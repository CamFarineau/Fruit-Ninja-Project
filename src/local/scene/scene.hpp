/*****************************************************************************\
 * 			IMI Projet - 4ETI
 *          Juin 2016 @CPE Lyon
 *			 --------------
 *	   Author :  Camille Farineau / Lucas Maigre / Prince Ngattai-Lam
 *
 *       ///   scene.hpp  ///
 *  Ce fichier contient tous les éléments relatifs à la scene 3D
 *  Il utilise beaucoup de classe et fonctions definies dans le dossier lib
 *  Il contient toutes les déclarations nécessaires, notamment celle de la 
 *  classe scene qui gère la scène 3D
 *
\*****************************************************************************/

/*****************************************************************************\
 * Inclusions et libraires nécessaires
\*****************************************************************************/
#pragma once

#ifndef SCENE_HPP
#define SCENE_HPP

#include <GL/gl.h>
#include <GL/glew.h>
#include <QWidget>
#include <vector>
#include <opencv2/opencv.hpp>
#include <list>

#include "../../lib/3d/mat3.hpp"
#include "../../lib/3d/vec3.hpp"
#include "../../lib/3d/vec2.hpp"
#include "../../lib/mesh/mesh.hpp"
#include "../../lib/opengl/mesh_opengl.hpp"
#include "../../lib/interface/camera_matrices.hpp"
#include "../../lib/3d/vec2.hpp"


/*****************************************************************************\
 * Define
\*****************************************************************************/

// Nombre de fruits différents (+ Bombe)
#define nb_fruits 11
// Taille du tableau de fruit
#define NB_MAX 100
// Nombre de vie intiale du joueur
#define NB_COEUR 3

// enum permettant de gérer le type de jeu 
 // type_score : jeu avec 3 vies
 // type_best : jeu avec temps limité 60s
 // type_final : fin du jeu
enum Type {type_score, type_best, type_final};

/*****************************************************************************\
 * Classe Fruit
 * Cette classe définit les objets de types Fruit
 * Elle permet de gérer l'emplacement des fruits, s'ils sont visibles etc.
 * Chaque fruit créer est une instance de cette classe 
 * Les attributs permettent de savoir où  est le fruit, s'il est visible
 * s'il a été coupé etc.
\*****************************************************************************/
class Fruit{
public :
		// Type de fruit : de 1 à NB_FRUITS
		// Permet la gestion du bon objet 3D en fonction du type de fruit
       int n;
       // Position du fruit sur l'écran (de -1 à +1 sur les axes x et y)
       cpe::vec2 pos;
       // Vecteur vitesse sur les deux axes x et y
       cpe::vec2 speed;
       // Vecteur à 3 composante contenant les informations sur l'orientation du fruit
       // Si le fruit est visible à chaque instant on lui fait subir une rotaton unitaire suivant 
       // l'axe définit ici
       cpe::vec3 orientation;
       // Pas d'intégration (pour le changement de position)
       // Ce pas est propre au fruit afin que ceux ci ai chacun une allure différentes pour effectuer leur courbe
       float dt;
       // Flag permettant de savoir si le fruit est visible (visiable = true) ou non (visible = false)
       bool visible;
       // Flag pour savoir si le fruit à été coupé (touch = true) ou non (touch = false)
       bool touch;

       // Constructeur par défaut
       // Lorsque le fruit est créer il est hors de l'écran (position en (-2;-2)), il a une vitesse nulle
       // une orientation suivant aucun axe, le pas d'intégration est nul, il n'est pas visible et pas coupé
       Fruit():
           n(-1),pos({-2.0f,-2.0f}),speed({0.0f,0.0f}),orientation({0.0f,0.0f,0.0f}),dt(0),visible(false),touch(false)
       {}


};
/*****************************************************************************\
 * Classe Coeur
 * Cette classe définit les objets Coeur
 * Elle permet de simplifier la gestion, notamment de l'affichage, des 3 coeurs
 * représentant les vies restantes de l'utilisateur
\*****************************************************************************/
class Coeur{
public :
		// Chaque coeur à une position (entre -1 et +1 pour l'axe x et y)
        cpe::vec2 pos;
        // Flag permettant de savoir si le coeur est visible
       bool visible;
       // Constructeur par défaut 
       Coeur():
           pos({-2.0f,-2.0f}),visible(true)
       {}


};

/*****************************************************************************\
 * Classe Point
 * Cette classe définit les objets Point
 * Chaque fois que le joueur coupe un fruit (ou un bombe) on affiche un +1 ou
 * -10 (pour une bombe) pour indiquer qu'il a bien coupé le fruit
 * Ces points s'affichent à l'endroit où le fruit à été coupé et pendant un
 * laps de temps
\*****************************************************************************/
class Point{
public :
	// Position pour l'affichage
    cpe::vec2 pos;
    // Flag pour savoir si le point est toujours visible
    bool visible;
    // Type de points : 0 -> +1 ou 1 -> -10
    int type;
    // Compteur qui se décremente au cours du temps
    // si le temps est supérieur à 0 alors on affiche 
    // la bonne image au bon endroit
    int temps;

    // Constructeur par défaut
    Point():
        pos({-2.0f,-2.0f}),visible(false),type(0),temps(0)
    {}
};

class myWidgetGL;


/*****************************************************************************\
 * Classe Scene
 * Cette classe permet de décrire la scène 3D
 * Il n'y a qu'une seule instance de cette classe à tout moment
 * Elle contient toutes les méthodes relatifs à l'affichage des différents objets
 * ainsi qu'au chargement des différentes images etc.
\*****************************************************************************/
class scene : public QWidget
{
public:

    scene();


    /** \brief Method called only once at the beginning (load off files ...) */
    void load_scene();

    /** \brief Method called at every frame */
    void draw_scene();

    /** Set the pointer to the parent Widget */
    void set_widget(myWidgetGL* widget_param);



private:

    /** Load a texture from a given file and returns its id */
    GLuint load_texture_file(std::string const& filename);

    /** Load common data such as shader and files */
    void load_common_data();

    /** Load fruits 3D files */
    void load_fruits();

    /** load background image*/
    void load_background();

    /** Load hearts 3D files */
    void load_coeurs();

	/** Load sword 3D file */
    void load_sword();

    /** Load numbers png files */
    void load_chiffres();

    /** Load numbers png files for the best score  */
    void load_best();

    /** Load numbers png files for the final score */
    void load_final();

    /** Load the png files for the welcome page of the game */
    void load_start();

    /** Load png files for the point (-1 and +10)*/
    void load_point();

    /** Draw of the different element (title, circle etc.) of the welcome page of the game */
    void draw_start();

    /** Draw the score of the users */
    void draw_score(enum Type);

    /** Draw the time remaining (for the type time of the game) */
    void draw_timer();

    /** Draw the point when the user cut a fruit (-1 or +10) */
    void draw_point();

    /** Draw the hearts */
    void draw_coeurs();

    /** Initialise correctement les différents points dans le tableau de points **/ 
    void init_point(bool neg,cpe::vec2 pos);

     /** Gère le score et permet d'intialiser les points dès que l'utilisateur coupe un fruit **/ 
    void gestion_score(Fruit& fruit);

    /** Mise à jour de la position du fruit **/ 
    void update_pos(Fruit &fruit);

    /** Load the recorder from the webcam */
    void start_webcam();
    /** Generate an openglGL texture from the OpenCV image */
    GLuint generate_texture_webcam(cv::Mat const& im);

    /** Load a shader and send the camera matrices */
    void prepare_shader(GLuint shader_id);
    
    /** Analyse de l'image (color tracking)**/ 
    void analyse_image(cv::Mat img);
    static void getObjectColor(int event, int x, int y, int flags, void *param);
    cpe::vec2 CalculObjectPos(cv::Mat image, cpe::vec2 objectNextPos, int nbPixels,cpe::vec2 objectPos);
    cpe::vec2 binarisation(cv::Mat image, int *nbPixels, int Color_choix);


    /** Affichage de la scene d'acceuil du jeu (gestion de l'affichage des fruit objet 3D)**/
    void draw_scene_acceuil();

    /** Affichage du meilleur score **/
    void draw_best();

    /** Affichage du score final **/
    void draw_final();

    /** Gestion de collision entre le fruit passé en paramètre et les bord de l'écran **/
    void collision_handling(Fruit& fruit);

    /** Gestion des collisions entres les fruits et l'épée **/
    void collision_fruit();

    /** Gestion des collisions entres les fruits et l'épée pour la scène d'acceuil du jeu**/
    void collision_fruit_accueil();

    /** Affichage de l'épée grâce à la position trouvé par l'algorithme de color tracking**/ 
    void draw_sword();

    /** Affichage des fruits **/
    void draw_fruit();

    /** Affichage du fond d'écran **/
    void draw_background();

    /** Initialisation de chaque instance de fruit créée **/
    void init_fruit();

    /** Initialisation des deux instances fruits créées pour la scene d'acceuil**/
    void init_accueil();

    /** Lecture du meilleur stocké dans le fichier score.txt **/
    void lecture_score();

    /** Ecriture du meilleur score dans le fichier score.txt **/
    void ecriture_score();

    /** Access to the parent object */
    myWidgetGL* pwidget;

    /** Default id for the texture (white texture) */
    GLuint texture_default;

    /** Texture with background */
    GLuint texture_background;

    /** Texture of title */
    GLuint texture_title;

    /** Texture of the time */
    GLuint texture_time;

    /** Texture for the life */
    GLuint texture_life;

    /** Texture of sword */
    GLuint texture_sword;

    /** Texture with fruits */
    GLuint texture_fruits[nb_fruits];

    /** Texture for points */
    GLuint texture_point[2];

    /** Texture with hearts */
    GLuint texture_coeurs;

    /** Texture for the score (numbers) */
    std::vector<GLuint>  texture_chiffres;

    /** Texture for the best score during game*/
    std::vector<GLuint> texture_chiffres_best;

    /** Texture for the best score at the end of the game */
    GLuint texture_best;

    /** Texture for the final score at the end of the game */
    GLuint texture_final;


    /** The id of the shader do draw meshes */
    GLuint shader_mesh;
    /** The id of the shader do draw the textured cube */
    GLuint shader_cube;

    /** The id of the shader do draw the textured sword */
    GLuint shader_sword;

    /** The id of the shader do draw the textured background */
    GLuint shader_background;

    /** The id of the shader do draw the textured title */
    GLuint shader_title;

    /** The id of the shader do draw the textured life */
    GLuint shader_life;

    /** The id of the shader do draw the textured time */
    GLuint shader_time;

    /** The id of the shader do draw the textured fruits */
    GLuint shader_fruits[nb_fruits];

    /** The id of the shader do draw the textured heart */
    GLuint shader_coeurs;

    /** The id of the shader do draw the textured points */
    GLuint shader_point;

    /** The id of the shader do draw the textured numbers (10 numbers from 0 to 9) */
    GLuint shader_chiffres[10];

    /** The shader dealing with the webcam in background */
    GLuint shader_webcam;

    /** The id of the shader do draw the textured best score*/
    GLuint shader_best;

    /** The id of the shader do draw the textured final score */
    GLuint shader_final;


    /** The current middle position of the first object choose with color tracking algorithm */
    cpe::vec2 middle;

    /** The current middle position of the second object choose with color tracking algorithm */
    cpe::vec2 middle_2;

    /** Store the middle position (of the first object) of the previous frames */
    std::list<cpe::vec2> store_middle;

    /** Store the middle position (of the second object) of the previous frames */
    std::list<cpe::vec2> store_middle_2;


    /** Mesh for the webcam in background */
    cpe::mesh mesh_camera;
    cpe::mesh_opengl mesh_camera_opengl;

    /** Mesh for the 3D cube */
    cpe::mesh mesh_cube;
    cpe::mesh_opengl mesh_cube_opengl;

    /** Mesh for the Background */
    cpe::mesh mesh_background;
    cpe::mesh_opengl mesh_background_opengl;

    /** Mesh for the title */
    cpe::mesh mesh_title;
    cpe::mesh_opengl mesh_title_opengl;

    /** Mesh for the time */
    cpe::mesh mesh_time;
    cpe::mesh_opengl mesh_time_opengl;

    /** Mesh for the life */
    cpe::mesh mesh_life;
    cpe::mesh_opengl mesh_life_opengl;

    /** Mesh for the 3D Sword */
    cpe::mesh mesh_sword;
    cpe::mesh_opengl mesh_sword_opengl;

    /** Mesh for the 3D fruits */
    cpe::mesh mesh_fruits[nb_fruits];
    cpe::mesh_opengl mesh_fruits_opengl[nb_fruits];

    /** Mesh for the points (-1 and +10) */
    cpe::mesh mesh_point[2];
    cpe::mesh_opengl mesh_point_opengl[2];

    /** Mesh for the hearts */
    cpe::mesh mesh_coeurs;
    cpe::mesh_opengl mesh_coeurs_opengl;

    /** Mesh for the numbers (from 0 to 9) */
    cpe::mesh mesh_chiffres[10];
    cpe::mesh_opengl mesh_chiffres_opengl[10];

    /** Mesh for the best score */
    cpe::mesh mesh_best;
    cpe::mesh_opengl mesh_best_opengl;

    /** Mesh for the final score */
    cpe::mesh mesh_final;
    cpe::mesh_opengl mesh_final_opengl;

    /** OpenCV webcam capture interface */
    cv::VideoCapture capture;

    /** Time parameter */
    int time_advance;

    /** Time parameter for the welcoming page **/ 
    int time_advance_accueil;

    // L'affichage s'effectue toutes les 25ms
    // Pour faire 1min = 60 sec = 2400 * 25ms
    int temps=2400;

    int unites[5]={100,10,1,10,1};

};

#endif
