/* tetris.cpp
Patrick Wang
301160793
CMPT 361 Asgn 1

Main Code

*/

//INCLUDES
#include "include/Angel.h"
#include "Pieces.h"
#include <iostream>
#include <cstdlib>
using namespace std;

//---Global Constants

const int cRows = 20;
const int cCols = 10;
const int blkSize = 25;		//square block
const int winWidth = cCols*blkSize + (cCols + 1);	//261
const int winHeight = cRows*blkSize + (cRows + 1);	//521

//Define Colours for blocks (Using http://www.tayloredmktg.com/rgb/ as a reference)
const vec4 TCOLOR[7][4] =
{
	{	//0 - RED
		vec4( 1.0, 0.0, 0.0 , 1.0),
	  	vec4( 1.0, 0.0, 0.0 , 1.0),
	  	vec4( 1.0, 0.0, 0.0 , 1.0),
	  	vec4( 1.0, 0.0, 0.0 , 1.0)
	},
	{	//1 - ORANGE
		vec4( 1.0, 0.647, 0.0 , 1.0),
	  	vec4( 1.0, 0.647, 0.0 , 1.0),
	  	vec4( 1.0, 0.647, 0.0 , 1.0),
	  	vec4( 1.0, 0.647, 0.0 , 1.0)
	},
	{	//2 - YELLOw
		vec4( 1.0, 1.0, 0.0 , 1.0),
	  	vec4( 1.0, 1.0, 0.0 , 1.0),
	  	vec4( 1.0, 1.0, 0.0 , 1.0),
	  	vec4( 1.0, 1.0, 0.0 , 1.0)
	},
	{	//3 - GREEN
		vec4( 0.196, 0.82, 0.196 , 1.0),
	  	vec4( 0.196, 0.82, 0.196 , 1.0),
	  	vec4( 0.196, 0.82, 0.196 , 1.0),
	  	vec4( 0.196, 0.82, 0.196 , 1.0)
	},
	{	//4 - BLUE
		vec4( 0.0, 0.0, 1.0 , 1.0),
	  	vec4( 0.0, 0.0, 1.0 , 1.0),
	  	vec4( 0.0, 0.0, 1.0 , 1.0),
	  	vec4( 0.0, 0.0, 1.0 , 1.0)
	},
	{	//5 - PURPLE
		vec4( 0.627, 0.125, 0.941 , 1.0),
	  	vec4( 0.627, 0.125, 0.941 , 1.0),
	  	vec4( 0.627, 0.125, 0.941 , 1.0),
	  	vec4( 0.627, 0.125, 0.941 , 1.0)
	},
	{	//6 - BLACK
		vec4( 0.0, 0.0, 0.0 , 1.0),
	  	vec4( 0.0, 0.0, 0.0 , 1.0),
	  	vec4( 0.0, 0.0, 0.0 , 1.0),
	  	vec4( 0.0, 0.0, 0.0 , 1.0)
	}
};

//Pieces data (positions, rotations)
static Pieces *tPieces;

//---Global Variables

//Current Piece's center location, piece type, rotation, color
int curX;
int curY;
int curPiece;
int curRot;
int curCol;

int curSpeed = 1;	//multiplier of the timer callback wait duration

int Board[cRows][cCols];		//filled condition of the cells in the board (value = color of cell) (black = empty)
vec4 cellV[cRows][cCols][4];	//cell grid position vertices 	STATIC
vec4 cellC[cRows][cCols][4];	//cell grid color vertices    	VARIABLE

//VAO allocation
GLuint cellVAO[cRows][cCols];	//20 rows x 10 col grid
GLuint curPieceVAO[4][4];		//16 blocks to check for each piece

//Shader program
GLuint program;

//block Position
struct bPos {
    vec4 bl;
    vec4 br;
    vec4 tl;
    vec4 tr;
};

//----------------------------------------------------------------------------
//----------------METHODS
//----------------------------------------------------------------------------

//returns 4 corner positions of a block (in terms of vec4 coords of screen) given the block's (x,y coords of grid)
struct bPos getBlockPos(int x, int y)
{
	struct bPos b;
	float lx = ((x*blkSize) + (x+1))/(float)winWidth * 2.0 - 1.0;						//left x
	float rx = (((x+1)*blkSize) + (x+1))/(float)winWidth * 2.0 - 1.0;					//right x
	float by = (((cRows-y-1)*blkSize) + (cRows-y-1))/(float)winHeight * 2.0 - 1.0;  		//bottom y
	float ty = (((cRows-y)*blkSize) + (cRows-y-1))/(float)winHeight * 2.0 - 1.0;    		//top y
	b.bl = vec4( lx, by, 0.0, 1.0 );															//bot left
	b.br = vec4( rx, by, 0.0, 1.0 );															//bot right
	b.tl = vec4( lx, ty, 0.0, 1.0 );															//top left
	b.tr = vec4( rx, ty, 0.0, 1.0 );															//top right
	return b;
}

//----------------------------------------------------------------------------

//PRE: A vertex array object is generated and binded
//Sets the VBO for the current vertex array object using 'vertices' as vertices data and 'color' as color data
//Uses the shader program 'program' on the VBO
void setupVBO(int vSize, vec4 *vertices, int cSize, vec4 *color)
{
	GLuint buffer;
	glGenBuffers( 1, &buffer );
	glBindBuffer( GL_ARRAY_BUFFER, buffer );
	glBufferData( GL_ARRAY_BUFFER, vSize + cSize, NULL, GL_STATIC_DRAW );
		glBufferSubData( GL_ARRAY_BUFFER, 0, vSize, vertices );
		glBufferSubData( GL_ARRAY_BUFFER, vSize, cSize, color );

	// set up vertex arrays
	GLuint vPosition = glGetAttribLocation( program, "vPosition" );
	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

	GLuint vColor = glGetAttribLocation( program, "vColor" );
	glEnableVertexAttribArray( vColor );
	glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(vSize));
}

//----------------------------------------------------------------------------

//Stores current Piece position & color data to the Board
void storeCurPiece(int px, int py, int piece, int rot, int color)
{
	for (int i=0; i<4; i++)
	{
		for (int j=0; j<4; j++)
		{
			if (tPieces->getBlockType(piece,rot,i,j) != 0)	//normal block
			{
				int boardX = px + j - 2;
				int boardY = py + i - 1;

				if ( (boardX >= 0) && (boardX < cCols) &&
					 (boardY >= 0) && (boardY < cRows) )	//valid cell on screen
				{
					Board[boardY][boardX] = color;
					cellC[boardY][boardX][0] = TCOLOR[color][1];
					cellC[boardY][boardX][1] = TCOLOR[color][2];
					cellC[boardY][boardX][2] = TCOLOR[color][3];
					cellC[boardY][boardX][3] = TCOLOR[color][3];

					glBindVertexArray(cellVAO[boardY][boardX]);
					setupVBO( sizeof(cellV[boardY][boardX]), cellV[boardY][boardX],
							  sizeof(cellC[boardY][boardX]), cellC[boardY][boardX]);
				}
			}
		}
	}
}


//----------------------------------------------------------------------------

//Stores the current piece's VAO in the buffer
void updateCurPiece( void )
{
	//generate piece VAO
	glGenVertexArrays(16, *curPieceVAO);

	//updating VAO and VBO for current piece
	for (int i=0; i<4; i++)
	{
		for (int j=0; j<4; j++)
		{
			//Only bind vertex array if there is a block there
			if (tPieces->getBlockType(curPiece,curRot,i,j) != 0)
			{
				//initilize vertex positions
				struct bPos curBlockPos = getBlockPos( curX + j - 2 , curY + i - 1 );
				vec4 curBlockVertices[] = {curBlockPos.bl,
										   curBlockPos.br,
										   curBlockPos.tl,
										   curBlockPos.tr};
				//initialize color positions
				vec4 color[4] = TCOLOR[curCol];

				glBindVertexArray(curPieceVAO[i][j]);
				setupVBO( sizeof(curBlockVertices), curBlockVertices, sizeof(color), color);
			}

		}
	}
}

//----------------------------------------------------------------------------

//Generate a new piece and start at the top
void newPiece( void )
{
	curX = 5;
	curY = -1;
	curPiece = (rand() % 7) + 0;	//random number between [0,6]
	curRot = (rand() % 4) + 0;      //random number between [0,3]
	curCol = (rand() % 6) + 0;		//random number between [0,5]
}

//----------------------------------------------------------------------------

//First initialization / Setup Cell grid
void init( void )
{

	//Initialize Board and set cell grid position/color vertices
	for (int i=0; i<cRows; i++)
	{
		for (int j=0; j<cCols; j++)
		{
			struct bPos cellPos = getBlockPos(j,i);
			cellV[i][j][0] = cellPos.bl;
			cellV[i][j][1] = cellPos.br;
			cellV[i][j][2] = cellPos.tl;
			cellV[i][j][3] = cellPos.tr;

			cellC[i][j][0] = TCOLOR[6][0];
			cellC[i][j][1] = TCOLOR[6][1];
			cellC[i][j][2] = TCOLOR[6][2];
			cellC[i][j][3] = TCOLOR[6][3];

			Board[i][j] = 6;
		}
	}

	//Generate cell VAB
	glGenVertexArrays(cRows*cCols, *cellVAO);

	//Load Shaders and use shader program
	program = InitShader( "vshader.glsl", "fshader.glsl" );
	glUseProgram( program );

	//Cell VAO setup (for each cRow x cCol cell)
	for (int i=0; i<cRows; i++)
	{
		for (int j=0; j<cCols; j++)
		{
			glBindVertexArray(cellVAO[i][j]);
			setupVBO( sizeof(cellV[i][j]), cellV[i][j], sizeof(cellC[i][j]), cellC[i][j]);
		}
	}

	newPiece();

	glClearColor( 0.5, 0.5, 0.5, 1.0 ); // grey background
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_BACK, GL_FILL);
}

//----------------------------------------------------------------------------
//--------------GLUT CALLBACKS

//Display Callback
void display( void )
{
	// Clear the window
	glClear( GL_COLOR_BUFFER_BIT );

	//Draw Board
	for (int i=0; i<cRows; i++)
	{
		for (int j=0; j<cCols; j++)
		{
			glBindVertexArray(cellVAO[i][j]);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
	}

	//Draw current Piece
	for (int i=0; i<4; i++)
	{
		for (int j=0; j<4; j++)
		{
			glBindVertexArray(curPieceVAO[i][j]);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
	}

	glBindVertexArray(0);

	glutSwapBuffers();
}

//----------------------------------------------------------------------------

//Keyboard Callback
void keyboard( unsigned char key, int x, int y )
{
    switch( key )
    {
		case 033: // Escape Key
			case 'q': case 'Q':
	    		exit( EXIT_SUCCESS );
	    	break;
    }
}

//Special Keys Down Callback
void specialkeysDown( int key, int x, int y )
{
    switch( key )
    {
		case GLUT_KEY_LEFT:				//Move left
			if (curX > 0) curX --;
	    	break;
	    case GLUT_KEY_RIGHT:			//Move right
	    	if (curX < 10) curX ++;
	    	break;
	    case GLUT_KEY_UP:				//Rotate 90 ccw
	    	curRot = (curRot + 1) % 4;
	    	break;
	    case GLUT_KEY_DOWN:				//Accelerate down
	    	curSpeed = 2;
	    	break;
    }
    glutPostRedisplay();
}

//Special Keys Up Callback
void specialkeysUp( int key, int x, int y )
{
    switch( key )
    {
	    case GLUT_KEY_DOWN:				//Reset acceleration
	    	curSpeed = 1;
	    	break;
    }
    glutPostRedisplay();
}

//Timer callback function
void timer( int val )
{

	updateCurPiece();

	if (curY <= 17)	//Haven't reached bottom.
	{
		curY += 1;
	} else			//reached bottom. Save piece and create new one at the top.
	{
	    storeCurPiece(curX,curY,curPiece,curRot,curCol);
		newPiece();
	}

    glutPostRedisplay();
	glutTimerFunc(200/curSpeed, timer, 1);
}

//Main Function
int main( int argc, char **argv )
{
	// Initialize glut library
	glutInit( &argc, argv );

	// Create the window
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE  );
	glutInitWindowSize( winWidth, winHeight);
	glutCreateWindow( "Tetris!" );

	// Initialize glew library
	glewInit();

	// Initialize /setup grid
	init();

	// Set callback functions
	glutDisplayFunc( display );
	glutKeyboardFunc( keyboard );
 	glutSpecialFunc( specialkeysDown );
 	glutSpecialUpFunc( specialkeysUp );
	glutTimerFunc(200/curSpeed,timer, 0);

	// Start the main loop
	glutMainLoop();

	return 0;
}
