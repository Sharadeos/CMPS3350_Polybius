//
//program: asteroids.cpp
//author:  Gordon Griesel
//date:    2014 - 2018
//mod spring 2015: added constructors
//This program is a game starting point for a 3350 project.
//
//
#include "classes.h"

//function prototypes
void init_opengl(void);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void physics();
void render();

// add prototypes of all external functions

void AdolfoValenciaPicture(int x, int y, GLuint textid);
void andrewH(int x, int y, GLuint textid, float i);
void creditsLuis(int x, int y, GLuint textid);
void showChrisRamirez(int x, int y, GLuint textid);
void josephG(int x, int y, GLuint textid);
void fighterPF(float* a, float* b, int x, int y);
void frigatePF(float* a, float* b, int x);
void squadronPF(float* a, float* b, int x, int y);
void missilePF(float* a, float* b, int x, int y);
void carrierPF(float* a, float* b, int x, int y);
//luis extern functions
void createAsteroid(Game *g, Global gl);
void createBullet(Game *g, Global gl, Object object);
void difficultyScaling(Game *g, Global gl);
//chris extern functions
void ALExplodeUpdate(ALenum param, float x, float y/*, ALfloat *z*/);
void playMusic();
//joey extern functions
void externalPhysics(Game *g, Global gl);
void externalRender(Game *g, Global gl);

// add png files name and create array based on # of pngs
//Image img("./images/bigfoot.png");
Image img[6] = {
"./images/bigfoot.png",
"./images/luis_3350.png",
"./images/IMG_Adolfo_Valencia.png",
"./images/chris_ramirez.png",
"./images/josephG.png",
"./images/blackhole.jpg"//macros
#define rnd() (((Flt)rand())/(Flt)RAND_MAX)
#define random(a) (rand()%a)
#define VecZero(v) (v)[0]=0.0,(v)[1]=0.0,(v)[2]=0.0
#define MakeVector(x, y, z, v) (v)[0]=(x),(v)[1]=(y),(v)[2]=(z)
#define VecCopy(a,b) (b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2]
#define VecDot(a,b)	((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])
#define VecSub(a,b,c) (c)[0]=(a)[0]-(b)[0]; \
						(c)[1]=(a)[1]-(b)[1]; \
						(c)[2]=(a)[2]-(b)[2]
};

//
Global gl;
//X Windows variables
class X11_wrapper {
private:
	Display *dpy;
	Window win;
	GLXContext glc;
public:
	X11_wrapper() { }
	X11_wrapper(int w, int h) {
		GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
		//GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, None };
		XSetWindowAttributes swa;
		setup_screen_res(gl.xres, gl.yres);
		dpy = XOpenDisplay(NULL);
		if (dpy == NULL) {
			std::cout << "\n\tcannot connect to X server" << std::endl;
			exit(EXIT_FAILURE);
		}
		Window root = DefaultRootWindow(dpy);
		XWindowAttributes getWinAttr;
		XGetWindowAttributes(dpy, root, &getWinAttr);
		int fullscreen=0;
		gl.xres = w;
		gl.yres = h;
		if (!w && !h) {
			//Go to fullscreen.
			gl.xres = getWinAttr.width;
			gl.yres = getWinAttr.height;
			//When window is fullscreen, there is no client window
			//so keystrokes are linked to the root window.
			XGrabKeyboard(dpy, root, False,
				GrabModeAsync, GrabModeAsync, CurrentTime);
			fullscreen=1;
		}
		XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
		if (vi == NULL) {
			std::cout << "\n\tno appropriate visual found\n" << std::endl;
			exit(EXIT_FAILURE);
		}
		Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
		swa.colormap = cmap;
		swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
			PointerMotionMask | MotionNotify | ButtonPress | ButtonRelease |
			StructureNotifyMask | SubstructureNotifyMask;
		unsigned int winops = CWBorderPixel|CWColormap|CWEventMask;
		if (fullscreen) {
			winops |= CWOverrideRedirect;
			swa.override_redirect = True;
		}
		win = XCreateWindow(dpy, root, 0, 0, gl.xres, gl.yres, 0,
			vi->depth, InputOutput, vi->visual, winops, &swa);
		//win = XCreateWindow(dpy, root, 0, 0, gl.xres, gl.yres, 0,
		//vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
		set_title();
		glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
		glXMakeCurrent(dpy, win, glc);
		show_mouse_cursor(0);
	}
	~X11_wrapper() {
		XDestroyWindow(dpy, win);
		XCloseDisplay(dpy);
	}
	void set_title() {
		//Set the window title bar.
		XMapWindow(dpy, win);
		XStoreName(dpy, win, "Asteroids template");
	}
	void check_resize(XEvent *e) {
		//The ConfigureNotify is sent by the
		//server if the window is resized.
		if (e->type != ConfigureNotify)
			return;
		XConfigureEvent xce = e->xconfigure;
		if (xce.width != gl.xres || xce.height != gl.yres) {
			//Window size did change.
			reshape_window(xce.width, xce.height);
		}
	}
	void reshape_window(int width, int height) {
		//window has been resized.
		setup_screen_res(width, height);
		glViewport(0, 0, (GLint)width, (GLint)height);
		glMatrixMode(GL_PROJECTION); glLoadIdentity();
		glMatrixMode(GL_MODELVIEW); glLoadIdentity();
		glOrtho(0, gl.xres, 0, gl.yres, -1, 1);
		set_title();
	}

	void setup_screen_res(const int w, const int h) {
		gl.xres = w;
		gl.yres = h;
	}
	void swapBuffers() {
		glXSwapBuffers(dpy, win);
	}
	bool getXPending() {
		return XPending(dpy);
	}
	XEvent getXNextEvent() {
		XEvent e;
		XNextEvent(dpy, &e);
		return e;
	}
	void set_mouse_position(int x, int y) {
		XWarpPointer(dpy, None, win, 0, 0, 0, 0, x, y);
	}
	void show_mouse_cursor(const int onoff) {
		if (onoff) {
			//this removes our own blank cursor.
			XUndefineCursor(dpy, win);
			return;
		}
		//vars to make blank cursor
		Pixmap blank;
		XColor dummy;
		char data[1] = {0};
		Cursor cursor;
		//make a blank cursor
		blank = XCreateBitmapFromData (dpy, win, data, 1, 1);
		if (blank == None)
			std::cout << "error: out of memory." << std::endl;
		cursor = XCreatePixmapCursor(dpy, blank, blank, &dummy, &dummy, 0, 0);
		XFreePixmap(dpy, blank);
		//this makes you the cursor. then set it using this function
		XDefineCursor(dpy, win, cursor);
		//after you do not need the cursor anymore use this function.
		//it will undo the last change done by XDefineCursor
		//(thus do only use ONCE XDefineCursor and then XUndefineCursor):
	}
} x11(0, 0);
Game* Game::instance = 0;
Game* Game::getInstance()
{
    if (instance == 0) {
        instance = new Game(gl.xres, gl.yres, Ship(0, 0, 0), Object(0, 100, 0));
    }
    return instance;
}
Game* g = Game::getInstance();




//==========================================================================
// M A I N
//==========================================================================
int main()
{
	system("xset r off");
	logOpen();
	init_opengl();
	srand(time(NULL));
	clock_gettime(CLOCK_REALTIME, &timePause);
	clock_gettime(CLOCK_REALTIME, &timeStart);
	x11.set_mouse_position(100,100);
	int done=0;



// MOVE

	playMusic();

	for (int i = 0; i < (*g).num_stars; i++) {
			(*g).stars[i][0] = (rand() % 359999)*.001; // maps to degrees
			(*g).stars[i][1] = (rand() % 179999)*.001;
			//(*g).stars[i][0] = i; // maps to degrees
			//(*g).stars[i][1] = i;
	}
// MOVE

	while (!done) {
		while (x11.getXPending()) {
			XEvent e = x11.getXNextEvent();
			x11.check_resize(&e);
			check_mouse(&e);
			done = check_keys(&e);
		}
		clock_gettime(CLOCK_REALTIME, &timeCurrent);
		timeSpan = timeDiff(&timeStart, &timeCurrent);
		timeCopy(&timeStart, &timeCurrent);


		//functions before render will not render on the setup_screen_res

		if (!(*g).show_credits) {
			physicsCountdown += timeSpan;
			while (physicsCountdown >= physicsRate) {
				physics();
				physicsCountdown -= physicsRate;
			}
			//difficultyScaling(g, gl);
		}

		render();
		x11.swapBuffers();
	}
	cleanup_fonts();
	logClose();
	return 0;
}
// in charge of initalizing image textures
void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, gl.xres, gl.yres);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//This sets 2D mode (no perspective)
	glOrtho(0, gl.xres, 0, gl.yres, -1, 1);
	//
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);
	glDisable(GL_CULL_FACE);
	//
	//Clear the screen to black
	glClearColor(0.0, 0.0, 0.0, 1.0);
	//Do this to allow fonts
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();

	glGenTextures(1, &gl.bigfootTexture);
	// Generate texture for each texture id
	int w = img[0].width;
	int h = img[0].height;

	// Bind texture for each texture id
	glBindTexture(GL_TEXTURE_2D, gl.bigfootTexture);
        //
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0,
		GL_RGB, GL_UNSIGNED_BYTE, img[0].data);

	glGenTextures(1, &gl.luisTexture);
	// Generate texture for each texture id
	w = img[1].width;
	h = img[1].height;

	// Bind texture for each texture id
	glBindTexture(GL_TEXTURE_2D, gl.luisTexture);
        //
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0,
		GL_RGB, GL_UNSIGNED_BYTE, img[1].data);

	glGenTextures(1, &gl.AdolfoTexture);

	w=img[2].width;
  h=img[2].height;
  glBindTexture (GL_TEXTURE_2D, gl.AdolfoTexture);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0,
     GL_RGB, GL_UNSIGNED_BYTE, img[2].data);

  glGenTextures(1, &gl.chrisTexture);
  w = img[3].width;
  h = img[3].width;
  glBindTexture(GL_TEXTURE_2D, gl.chrisTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0,
     GL_RGB, GL_UNSIGNED_BYTE, img[3].data);

  glGenTextures(1, &gl.josephTexture);
  w = img[4].width;
  h = img[4].width;
  glBindTexture(GL_TEXTURE_2D, gl.josephTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0,
     GL_RGB, GL_UNSIGNED_BYTE, img[4].data);

 	glGenTextures(1, &gl.blackholeTexture);
  w = img[5].width;
  h = img[5].width;
  glBindTexture(GL_TEXTURE_2D, gl.blackholeTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0,
    GL_RGB, GL_UNSIGNED_BYTE, img[5].data);



}

void normalize2d(Vec v)
{
	Flt len = v[0]*v[0] + v[1]*v[1];
	if (len == 0.0f) {
		v[0] = 1.0;
		v[1] = 0.0;
		return;
	}
	len = 1.0f / sqrt(len);
	v[0] *= len;
	v[1] *= len;
}

void check_mouse(XEvent *e)
{
	/*
	//Did the mouse move?
	//Was a mouse button clicked?
	static int savex = 0;
	static int savey = 0;
	//
	static int ct=0;
	//std::cout << "m" << std::endl << std::flush;
	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button is down
			//a little time between each bullet
					//createBullet(g, gl, (*g).ship);


		}
		if (e->xbutton.button==3) {
			//Right button is down
		}
	}
	//keys[XK_Up] = 0;
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		//Mouse moved
		int xdiff = savex - e->xbutton.x;
		int ydiff = savey - e->xbutton.y;
		if (++ct < 10)
			return;
		//std::cout << "savex: " << savex << std::endl << std::flush;
		//std::cout << "e->xbutton.x: " << e->xbutton.x << std::endl <<
		//std::flush;
		if (xdiff > 0) {
			//std::cout << "xdiff: " << xdiff << std::endl << std::flush;
			(*g).ship.angle += 0.05f * (float)xdiff;
			if ((*g).ship.angle >= 360.0f)
				(*g).ship.angle -= 360.0f;
		}
		else if (xdiff < 0) {
			//std::cout << "xdiff: " << xdiff << std::endl << std::flush;
			(*g).ship.angle += 0.05f * (float)xdiff;
			if ((*g).ship.angle < 0.0f)
				(*g).ship.angle += 360.0f;
		}
		if (ydiff > 0) {
			//apply thrust
			//convert ship angle to radians
			Flt rad = (((*g).ship.angle+90.0) / 360.0f) * PI * 2.0;
			//convert angle to a vector
			Flt xdir = cos(rad);
			Flt ydir = sin(rad);
			(*g).ship.vel[0] += xdir * (float)ydiff * 0.01f;
			(*g).ship.vel[1] += ydir * (float)ydiff * 0.01f;
            Flt speed = sqrt((*g).ship.vel[0]*(*g).ship.vel[0]+
					(*g).ship.vel[1]*(*g).ship.vel[1]);
			if (speed > 10.0f) {
				speed = 10.0f;
				normalize2d((*g).ship.vel);
				(*g).ship.vel[0] *= speed;
				(*g).ship.vel[1] *= speed;
			}
			(*g).mouseThrustOn = true;
			clock_gettime(CLOCK_REALTIME, &(*g).mouseThrustTimer);
		}
		x11.set_mouse_position(100,100);
		savex=100;
		savey=100;
	}
	*/
}


int check_keys(XEvent *e)
{
	int key = XLookupKeysym(&e->xkey, 0);
	if (e->type == KeyRelease) {
		gl.keyhits[key%KEYS] = 0;
	}
	if (e->type == KeyPress) {
		//std::cout << "press" << std::endl;
		gl.keyhits[key%KEYS] = 1;
		if (key == XK_Escape)
			return 1;
	}
	return 0;

	/*
	//keyboard input?
	static int shift=0;
	int key = (XLookupKeysym(&e->xkey, 0) & 0x0000ffff);
	//Log("key: %i\n", key);
	if (e->type == KeyRelease) {
		gl.keys[key]=0;
		if (key == XK_Shift_L || key == XK_Shift_R)
			shift=0;
		return 0;
	}
	if (e->type == KeyPress) {
		//std::cout << "press" << std::endl;
		gl.keys[key]=1;
		if (key == XK_Shift_L || key == XK_Shift_R) {
			shift=1;
			return 0;
		}
	} else {
		return 0;
	}
	if (shift){}
	switch (key) {
		case XK_Escape:
			return 1;
		case XK_c:
			(*g).show_credits = !(*g).show_credits;
			break;
		case XK_q:
			createAsteroid(g, gl);
			break;
		case XK_e:
			(*g).ship.powerLevel++;
			break;
		case XK_Down:
			break;
		case XK_equal:
			break;
		case XK_minus:
			break;
	}
	return 0;
	*/
}

void deleteAsteroid(Game *g, Asteroid *node)
{
	//Remove a node from doubly-linked list
	//Must look at 4 special cases below.
	if (node->prev == NULL) {
		if (node->next == NULL) {
			//only 1 item in list.
			g->ahead = NULL;
		} else {
			//at beginning of list.
			node->next->prev = NULL;
			g->ahead = node->next;
		}
	} else {
		if (node->next == NULL) {
			//at end of list.
			node->prev->next = NULL;
		} else {
			//in middle of list.
			node->prev->next = node->next;
			node->next->prev = node->prev;
		}
	}
	delete node;
	node = NULL;
}
/*
void buildAsteroidFragment(Asteroid *ta, Asteroid *a)
{
	//build ta from a
	ta->nverts = 8;
	ta->radius = a->radius / 2.0;
	Flt r2 = ta->radius / 2.0;
	Flt angle = 0.0f;
	Flt inc = (PI * 2.0) / (Flt)ta->nverts;
	for (int i=0; i<ta->nverts; i++) {
		ta->vert[i][0] = sin(angle) * (r2 + rnd() * ta->radius);
		ta->vert[i][1] = cos(angle) * (r2 + rnd() * ta->radius);
		angle += inc;
	}
	ta->pos[0] = a->pos[0] + rnd()*10.0-5.0;
	ta->pos[1] = a->pos[1] + rnd()*10.0-5.0;
	ta->pos[2] = 0.0f;
	ta->angle = 0.0;
	ta->rotate = a->rotate + (rnd() * 4.0 - 2.0);
	ta->color[0] = 0.8;
	ta->color[1] = 0.8;
	ta->color[2] = 0.7;
	ta->vel[0] = a->vel[0] + (rnd()*2.0-1.0);
	ta->vel[1] = a->vel[1] + (rnd()*2.0-1.0);
	ta->currentHealth = a->maxHealth;
	//std::cout << "frag" << std::endl;
}
*/
void physics()
{
	//Update ship position

	externalPhysics(g, gl);

}


void drawObject(Object & rend_object)
{

	int sizeofObject = 50;
	float tempValue = 0;

	float e[3];
	e[0] = 0;
	e[1] = rend_object.polar[1];
	e[2] = rend_object.polar[2];

    if (e[1] < 0) {
        e[1] = 360 + e[1];
    }
    float s[2];
	s[0] = (*g).ship.angle[0];
	s[1] = (*g).ship.angle[1];
	float low, high;
	low = s[0] - 60;
	high = s[0] + 60;
	high -= low;
    e[1] -= low;
    if (e[1] > 360) {
        e[1] = e[1] - 360;
    }
    float x, y;

		x = ((high - e[1])/120)*gl.xres;
  	y = ((s[1] + 45 - e[2])/90)*gl.yres;



		//Scale max at the right edge of the setup_screen

		rend_object.xScale = ((high - e[1])/60);
		rend_object.yScale = ((s[1] + 45 - e[2])/45);

		if (rend_object.xScale  > 1.0) {
			tempValue = rend_object.xScale - 1.0;
			rend_object.xScale = 1.0;
			rend_object.xScale = rend_object.xScale - tempValue;
		}

		if (rend_object.yScale  > 1.0) {
			tempValue = rend_object.yScale - 1.0;
			rend_object.yScale = 1.0;
			rend_object.yScale = rend_object.yScale - tempValue;
		}


		float distanceScale = 12/rend_object.polar[0];

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glBegin(GL_POLYGON);
			/*
			 glVertex2i(50,100);
			 glVertex2i(100,100);
			 glVertex2i(100,150);
			 glVertex2i(50,150);
			 */
			 glVertex2i(x-(40*rend_object.xScale*distanceScale),y-(40*rend_object.yScale*distanceScale));
			 glVertex2i(x-(40*rend_object.xScale*distanceScale),y+(40*rend_object.yScale*distanceScale));
			 glVertex2i(x+(40*rend_object.xScale*distanceScale),y-(40*rend_object.yScale*distanceScale));
			 glVertex2i(x+(40*rend_object.xScale*distanceScale),y+(40*rend_object.yScale*distanceScale));

			 //glVertex2f(50, 100);
			 //glVertex2i(100,100);
			 //glVertex2i(100,150);
			 //glVertex2i(50,150);
		glEnd();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);






		glColor3f(1.0f, 0.0f, 0.0f);
		glBegin(GL_POINTS);
		//glVertex2f(object.drawSize[0], object.drawSize[1]);
		glEnd();
}


void drawBullet(Bullet rend_object, float xScale, float yScale)
{

	int sizeofObject = 50;
	float e[3];
	e[0] = 0;
	e[1] = rend_object.polar[1];
	e[2] = rend_object.polar[2];

    if (e[1] < 0) {
        e[1] = 360 + e[1];
    }
    float s[2];
	s[0] = (*g).ship.angle[0];
	s[1] = (*g).ship.angle[1];
	float low, high;
	low = s[0] - 60;
	high = s[0] + 60;
	high -= low;
  e[1] -= low;
  if (e[1] > 360) {
      e[1] = e[1] - 360;
  }
  float x, y;

	x = ((high - e[1])/120)*gl.xres;
	y = ((s[1] + 45 - e[2])/90)*gl.yres;

  glColor3fv(rend_object.color);
  glPushMatrix();
  glBegin(GL_POLYGON);
	glVertex2i(x-5,y);
	glVertex2i(x,y+5);
	glVertex2i(x+5,y);
	glVertex2i(x,y-5);
	glEnd();
  glPopMatrix();
/*
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glBegin(GL_POLYGON);
			 glVertex2i(50,100);
			 glVertex2i(100,100);
			 glVertex2i(100,150);
			 glVertex2i(50,150);
			 glVertex2i(x+rend_object.drawSize[0]-sizeofObject*xScale,y+rend_object.drawSize[1]+sizeofObject*yScale);
			 glVertex2i(x+rend_object.drawSize[0]+sizeofObject*xScale,y+rend_object.drawSize[1]+sizeofObject*yScale);
			 glVertex2i(x+rend_object.drawSize[0]+sizeofObject*xScale,y+rend_object.drawSize[1]-sizeofObject*yScale);
			 glVertex2i(x+rend_object.drawSize[0]-sizeofObject*xScale,y+rend_object.drawSize[1]-sizeofObject*yScale);

			 //glVertex2f(50, 100);
			 //glVertex2i(100,100);
			 //glVertex2i(100,150);
			 //glVertex2i(50,150);
		glEnd();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glColor3f(1.0f, 0.0f, 0.0f);
		glBegin(GL_POINTS);
		glVertex2f(object.drawSize[0], object.drawSize[1]);
		glEnd();
*/
}



void render()
{

	glClear(GL_COLOR_BUFFER_BIT);
	if (!(*g).show_credits) {

		drawObject((*g).object);


		for (int i=0; i< (*g).nbullets; i++) {
			Bullet *b = & (*g).barr[i];
		    drawBullet( (*g).barr[i], 1, 1);
	    }
				//Draw the asteroids
				{
					Asteroid *a =  (*g).ahead;
					while (a) {
						//Loig("draw asteroid...\n");
						glColor3fv(a->color);
						glPushMatrix();
						glTranslatef(a->pos[0], a->pos[1], a->pos[2]);
						glRotatef(a->angle, 0.0f, 0.0f, 1.0f);
						glBegin(GL_LINE_LOOP);
						//Log("%i verts\n",a->nverts);
						for (int j=0; j<a->nverts; j++) {
							glVertex2f(a->vert[j][0], a->vert[j][1]);
						}
						glEnd();
						//glBegin(GL_LINES);
						//	glVertex2f(0,   0);
						//	glVertex2f(a->radius, 0);
						//glEnd();
						glPopMatrix();
						glColor3f(1.0f, 0.0f, 0.0f);
						glBegin(GL_POINTS);
						glVertex2f(a->pos[0], a->pos[1]);
						glEnd();
						a = a->next;
					}
				}
				//-------------------------------------------------------------------------
				/*Draw the bullets
				for (int i=0; i<(*g).nbullets; i++) {
					Bullet *b = &(*g).barr[i];
					//Log("draw bullet...\n");
					glColor3f(0.0, 1.0, 0.0);
					glBegin(GL_POINTS);
					glVertex2f(b->pos[0],      b->pos[1]);
					glVertex2f(b->pos[0]-1.0f, b->pos[1]);
					glVertex2f(b->pos[0]+1.0f, b->pos[1]);
					glVertex2f(b->pos[0],      b->pos[1]-1.0f);
					glVertex2f(b->pos[0],      b->pos[1]+1.0f);
					glColor3f(0.8, 0.8, 0.8);
					glVertex2f(b->pos[0]-1.0f, b->pos[1]-1.0f);
					glVertex2f(b->pos[0]-1.0f, b->pos[1]+1.0f);
					glVertex2f(b->pos[0]+1.0f, b->pos[1]-1.0f);
					glVertex2f(b->pos[0]+1.0f, b->pos[1]+1.0f);
					glEnd();
				}
			    */
					externalRender(g, gl);
			  	//ggprint8b(&r, 16, 0x00ffff00, "(object render x=%.1f,object render y=%.1f)", (((*g).ship.angle[0] + 60 - object.polar[1])/120)*gl.xres,  (((*g).ship.angle[1] + 45 - object.polar[2])/90)*gl.yres);
}


	if ((*g).show_credits) {
	    (*g).mtext -= .02;
	    andrewH(.5*gl.xres, .9*gl.yres, gl.bigfootTexture,(*g).mtext);
  	  creditsLuis(.5*gl.xres, .7*gl.yres, gl.luisTexture);
	    AdolfoValenciaPicture(.5*gl.xres, .5*gl.yres, gl.AdolfoTexture);
    	showChrisRamirez(.5*gl.xres, .3*gl.yres, gl.chrisTexture);
	    josephG(.5*gl.xres, .1*gl.yres, gl.josephTexture);
        // function calls for everyone with parameters
	}
}
