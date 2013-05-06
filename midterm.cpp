//cs335 Spring 2013
//midterm.cpp
//Pascual Martinez

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <GL/glfw.h>
#include <unistd.h>

#define rnd() (double)rand()/(double)RAND_MAX

void init(void);
int init_glfw(void);
void init_opengl(void);
void init_grid(void);
void render(void);
void GLFWCALL mouse_click(int button, int action);
void check_mouse(void);
void get_grid_center(const int i, const int j, int cent[2]);
int xres=640;
int yres=480;
int lcount = 0,rcount = 0;

typedef struct t_grid {
	char contents;
	bool mouse_hover;
	bool mark;
	bool check;
	int row;
	int col;
} Grid;
Grid **grid;
int grid_dim;
int board_dim;
int qsize;
char winner = '\0';

GLuint Htexture;
GLuint Vtexture;
GLuint Wintexture;
GLuint loadBMP(const char *imagepath);
void process_turn(Grid &cell, char value, int grid_dim);

int main(int argc, char *argv[])
{
	printf("What size would you like the board to be?: ");
	int tmp = scanf("%d",&grid_dim);
	if (init_glfw()) {
		exit(EXIT_FAILURE);
	}
	init_opengl();
	init();
	srand((unsigned int)time(NULL));
NewGame:
	init_grid();
	while(1) {
		if (winner !='\0'){
		    render();
		    glfwSwapBuffers();
		    winner ='\0';
		    sleep(1);
		    goto NewGame;
		}
		check_mouse();
		render();
		glfwSwapBuffers();
		if (glfwGetKey(GLFW_KEY_ESC) == GLFW_PRESS) break;
		if (!glfwGetWindowParam(GLFW_OPENED)) break;
	}
	glfwTerminate();
	printf("Total moves for Vert is: %d\n",lcount);
	printf("Total moves for Hori is: %d\n",rcount);
	exit(EXIT_SUCCESS);
}

void init_grid(void)
{
	// Allocate grid memory, initialize cells.
	int i, j;
	grid = (Grid**) calloc(grid_dim, sizeof(Grid*));
	for(i=0; i < grid_dim; i++) {
		grid[i] = (Grid*) calloc(grid_dim, sizeof(Grid));
		for(j=0; j < grid_dim; j++) {
			grid[i][j].contents = '\0';
			grid[i][j].mouse_hover = false;
			grid[i][j].mark = false;
			grid[i][j].check = false;
			grid[i][j].row = i;
			grid[i][j].col = j;
		}
	}
}

int init_glfw(void)
{
	int nmodes;
	GLFWvidmode glist[256];
	if (!glfwInit()){
		printf("Failed to initialize GLFW\n");
		return 1;
	}
	//get the monitor native full-screen resolution
	nmodes = glfwGetVideoModes(glist, 250);
	xres = glist[nmodes-1].Width;
	yres = glist[nmodes-1].Height;
	//create a window
	//if (!glfwOpenWindow(xres, yres, 0, 0, 0, 0, 0, 0, GLFW_WINDOW)) {
	if (!glfwOpenWindow(xres,yres,8,8,8,0,32,0,GLFW_FULLSCREEN)) {
		glfwTerminate();
		return 1;
	}
	glfwSetWindowTitle("Game Board and Grid");
	glfwSetWindowPos(0, 0);
	//make sure we see the escape key pressed
	glfwEnable(GLFW_STICKY_KEYS);
	glfwSetMouseButtonCallback(mouse_click);
	glfwEnable( GLFW_MOUSE_CURSOR );
	//enable vertical sync (on cards that support it)
	glfwSwapInterval(1);
	return 0;
}

void init_opengl(void)
{
	//OpenGL initialization
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_COLOR_MATERIAL);
	//
	//choose one of these
	//glShadeModel(GL_FLAT);
	glShadeModel(GL_SMOOTH);
	glDisable(GL_LIGHTING);
	glBindTexture(GL_TEXTURE_2D, 0);
	//
	glEnable(GL_TEXTURE_2D);
	Htexture = loadBMP("H.bmp");
	Vtexture = loadBMP("V.bmp");
	glBindTexture(GL_TEXTURE_2D, 0);
}

void init(void)
{
	board_dim = yres - 200;
	//make board dim divisible by grid_dim
	board_dim -= (board_dim % grid_dim);
	int bq;
	//quad upper-left corner
	//bq is the width of one grid section
	bq = (board_dim / grid_dim);
	qsize = (bq-10) / 2;
	//
	//notes:
	//This code is not generic.
	//A goal for this project is to make your board-size generic.
	//Allow the user to select the grid dimensions.
	//For instance, 8x8 board, or 20x20 board.
	//Maybe get this parameter at the command-line.
}

void check_mouse(void)
{
	static int sx=0,sy=0;
	int x,y;
	int i,j;
	int cent[2];
	glfwGetMousePos(&x, &y);
	//reverse the y position
	y = yres - y;
	if (x == sx && y == sy) return;
	sx=x;
	sy=y;
	//
	//is the mouse over any grid squares?
	//
	for (i=0; i<grid_dim; i++) {
		for (j=0; j<grid_dim; j++) {
			grid[i][j].mouse_hover=false;
		}
	}
	for (i=0; i<grid_dim; i++) {
		for (j=0; j<grid_dim; j++) {
			get_grid_center(i,j,cent);
			if (x >= cent[0]-qsize &&
				x <= cent[0]+qsize &&
				y >= cent[1]-qsize &&
				y <= cent[1]+qsize) {
				grid[i][j].mouse_hover=true;
				break;
				//You could do a return here.
				//If more code is added below, a return
				//would cause you to exit too early.
			}
		}
		if (grid[i][j].mouse_hover) break;
	}
}

void GLFWCALL mouse_click(int button, int action)
{
	int x,y;
	if (action == GLFW_PRESS) {
		int i,j=0;
		//center of a grid
		int cent[2];
		glfwGetMousePos(&x, &y);
		//reverse the y position
		y = yres - y;
		for (i=0; i<grid_dim; i++) {
			for (j=0; j<grid_dim; j++) {
				get_grid_center(i,j,cent);
				if (x >= cent[0]-qsize &&
					x <= cent[0]+qsize &&
					y >= cent[1]-qsize &&
					y <= cent[1]+qsize) {
					if (button == GLFW_MOUSE_BUTTON_LEFT){
						process_turn(grid[i][j], 'V', grid_dim);
						lcount++;
					}
					if (button == GLFW_MOUSE_BUTTON_RIGHT){
						process_turn(grid[i][j], 'H', grid_dim);
						rcount++;
					}
					return;
				}
			}
		}
	}
}

void get_grid_center(const int i, const int j, int cent[2])
{
	int b2 = board_dim/2;
	int screen_center[2] = {xres/2, yres/2};
	int s0 = screen_center[0];
	int s1 = screen_center[1];
	int bq;
	int quad[2];
	board_dim -= (board_dim % grid_dim);
	bq = (board_dim / grid_dim);
	
	quad[0] = s0-b2;
	quad[1] = s1-b2;
	cent[0] = quad[0] + bq/2;
	cent[1] = quad[1] + bq/2;
	cent[0] += (bq * j);
	cent[1] += (bq * i);
}

void render(void)
{
	int i,j;
	int b2 = board_dim/2;
	int screen_center[2] = {xres/2, yres/2};
	int s0 = screen_center[0];
	int s1 = screen_center[1];
	int bq, bp;
	//quad upper-left corner
	int quad[2], saveq0;
	//center of a grid
	int cent[2];
	//bq is the width of one grid section
	bq = (board_dim / grid_dim);
	
	glViewport(0, 0, xres, yres);
	glClearColor(0.1f, 0.2f, 0.3f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	//init matrices
	glMatrixMode (GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	glOrtho(0, xres, 0, yres, -1, 1);
	glColor3f(0.8f, 0.6f, 0.2f);
	
	glBegin(GL_QUADS);
		glVertex2i(s0-b2, s1-b2);
		glVertex2i(s0-b2, s1+b2);
		glVertex2i(s0+b2, s1+b2);
		glVertex2i(s0+b2, s1-b2);
	glEnd();
	//draw grid lines
	//vertical
	glColor3f(0.1f, 0.1f, 0.1f);
	bp = s0-b2;
	glLineWidth(2);
	glBegin(GL_LINES);
		bp += bq;
		glVertex2i(bp, s1-b2);
		glVertex2i(bp, s1+b2);
		bp += bq;
		glVertex2i(bp, s1-b2);
		glVertex2i(bp, s1+b2);
		bp += bq;
		glVertex2i(bp, s1-b2);
		glVertex2i(bp, s1+b2);
	glEnd();
	//horizontal
	glColor3f(0.2f, 0.2f, 0.2f);
	bp = s1-b2;
	glBegin(GL_LINES);
		bp += bq;
		glVertex2i(s0-b2, bp);
		glVertex2i(s0+b2, bp);
		bp += bq;
		glVertex2i(s0-b2, bp);
		glVertex2i(s0+b2, bp);
		bp += bq;
		glVertex2i(s0-b2, bp);
		glVertex2i(s0+b2, bp);
	glEnd();
	glLineWidth(1);
	//Draw square in cener of each grid
	for (i=0; i<grid_dim; i++) {
		for (j=0; j<grid_dim; j++) {
			get_grid_center(i,j,cent);
			glColor3f(0.5f, 0.1f, 0.1f);
			if (grid[i][j].mouse_hover) {
				glColor3f(1.0f, 1.0f, 0.0f);
			}
			glBindTexture(GL_TEXTURE_2D, 0);
			if (grid[i][j].mark)               glBindTexture(GL_TEXTURE_2D, Wintexture);
			else if (grid[i][j].contents=='V') glBindTexture(GL_TEXTURE_2D, Vtexture);
			else if (grid[i][j].contents=='H') glBindTexture(GL_TEXTURE_2D, Htexture);
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f); glVertex2i(cent[0]-qsize,cent[1]-qsize);
				glTexCoord2f(0.0f, 1.0f); glVertex2i(cent[0]-qsize,cent[1]+qsize);
				glTexCoord2f(1.0f, 1.0f); glVertex2i(cent[0]+qsize,cent[1]+qsize);
				glTexCoord2f(1.0f, 0.0f); glVertex2i(cent[0]+qsize,cent[1]-qsize);
			glEnd();
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
}

GLuint loadBMP(const char *imagepath)
{
	//When you create your texture files, please specify
	//type: BMP
	//color depth: 24-bit
	unsigned int retval;
	unsigned char header[54];
	//Each BMP file begins by a 54-bytes header
	//Position in the file where the actual data begins
	unsigned int dataPos;
	unsigned int width, height;
	unsigned int imageSize;
	// = width*height*3
	//RGB data will go in this
	unsigned char *data;
	//
	printf("loadBMP(%s)...\n",imagepath);
	//Log("opening file **%s**\n",imagepath);
	FILE * file = fopen(imagepath,"r");
	if (!file) {
		printf("Image could not be opened\n");
		return 0;
	}
	if (fread(header, 1, 54, file)!=54) {
		// If not 54 bytes read : problem
		printf("Not a correct BMP file\n");
		return 0;
	}
	if (header[0]!='B' || header[1]!='M') {
		printf("Not a correct BMP file\n");
		return 0;
	}
	dataPos   = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width     = *(int*)&(header[0x12]);
	height    = *(int*)&(header[0x16]);
	//Some BMP files are misformatted, guess missing information
	if (imageSize==0) imageSize=width*height*3;
	if (dataPos==0) dataPos=54;
	data = (unsigned char *)malloc(imageSize+1);
	//Read the actual data from the file into the buffer
	retval = fread(data,1,imageSize,file);
	fclose(file);
	
	#define GL_BGR 0x80E0
	//Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	free(data);
	return textureID;
}

bool path_vert (Grid &cell, Grid** grid, int size)
{
	cell.mark = true;
	if (cell.row == size - 1)
		return true;

	int row_min = cell.row - 1;
	if (row_min < 0)
		row_min = 0;

	int row_max = cell.row + 1;
	if (row_max >= size)
		row_max = size - 1;

	int col_min = cell.col - 1;
	if (col_min < 0)
		col_min = 0;

	int col_max = cell.col + 1;
	if (col_max >= size)
		col_max = size - 1;

	for (int row = row_max; row >= row_min; row--) {
		for (int col = col_min; col <= col_max; col++) {
			if (grid[row][col].contents == 'V' && grid[row][col].check == false) {

				grid[row][col].check = true;
				bool good = path_vert (grid[row][col], grid, size);

				if (!good)
					grid[row][col].mark = false;

				if (grid[row][col].mark == true)
					return good;
			}
		}
	}
	return false;
}

bool path_horiz (Grid &cell, Grid** grid, int size)
{
	cell.mark = true;
	if (cell.col == size - 1)
		return true;

	int row_min = cell.row - 1;
	if (row_min < 0)
		row_min = 0;

	int row_max = cell.row + 1;
	if (row_max >= size)
		row_max = size - 1;

	int col_min = cell.col - 1;
	if (col_min < 0)
		col_min = 0;

	int col_max = cell.col + 1;
	if (col_max >= size)
		col_max = size - 1;

	for (int row = row_min; row <= row_max; row++) {
		for (int col = col_max; col >= col_min; col--) {
			if (grid[row][col].contents == 'H' && grid[row][col].check == false) {

				grid[row][col].check = true;
				bool good = path_horiz (grid[row][col], grid, size);

				if (!good)
					grid[row][col].mark = false;

				if (grid[row][col].mark == true)
						return good;
			}
		}
	}
	return false;
}

// Process player's choice
void process_turn(Grid &cell, char value, int grid_dim) {
	int i, j;
	if(cell.contents) 
		return;
	cell.contents = value;

	for(i=0; i < grid_dim; i++) {
		for(j=0; j < grid_dim; j++) {
			grid[i][j].mark = false;
			grid[i][j].check = false;
		}
	}

	if(value == 'H') {
		for(i=0; i < grid_dim; i++) {
			if(grid[i][0].contents == 'H' && path_horiz(grid[i][0], grid, grid_dim)) {
				winner = 'H';
				printf("Winner: H\n");
				return;
			}
		}
	}
	if(value == 'V') {
		for(i=0; i < grid_dim; i++) {
			if(grid[0][i].contents == 'V' && path_vert(grid[0][i], grid, grid_dim)) {
				winner = 'V';
				printf("Winner: V\n");
				return;
			}
		}
	}
}


