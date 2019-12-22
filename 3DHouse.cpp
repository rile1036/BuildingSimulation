
#include "3Dbuild.h"

// �޸𸮿� TGA ������ �ε��ϴ� �Լ�
bool load_TGA(TextureImage *texture, char *filename)
{
	GLubyte		TGAheader[12] = { 0,0,2,0,0,0,0,0,0,0,0,0 };					// Uncompressed TGA Header
	GLubyte		TGAcompare[12];												// Used To Compare TGA Header
	GLubyte		header[6];													// First 6 Useful Bytes From The Header
	GLuint		bytesPerPixel;												// Holds Number Of Bytes Per Pixel Used In The TGA File
	GLuint		imageSize;													// Used To Store The Image Size When Setting Aside Ram
	GLuint		temp;														// Temporary Variable
	GLuint		type = GL_RGBA;												// Set The Default GL Mode To RBGA (32 BPP)

	FILE *file = fopen(filename, "rb");										// Open The TGA File

	if (file == NULL ||														// Does File Even Exist?
		fread(TGAcompare, 1, sizeof(TGAcompare), file) != sizeof(TGAcompare) ||	// Are There 12 Bytes To Read?
		memcmp(TGAheader, TGAcompare, sizeof(TGAheader)) != 0 ||	// Does The Header Match What We Want?
		fread(header, 1, sizeof(header), file) != sizeof(header))				// If So Read Next 6 Header Bytes
	{
		if (file == NULL)													// Did The File Even Exist? *Added Jim Strong*
			return FALSE;													// Return False
		else																// Otherwise
		{
			fclose(file);													// If Anything Failed, Close The File
			return FALSE;													// Return False
		}
	}

	texture->width = header[1] * 256 + header[0];							// Determine The TGA Width	(highbyte*256+lowbyte)
	texture->height = header[3] * 256 + header[2];							// Determine The TGA Height	(highbyte*256+lowbyte)

	if (texture->width <= 0 ||												// Is The Width Less Than Or Equal To Zero
		texture->height <= 0 ||												// Is The Height Less Than Or Equal To Zero
		(header[4] != 24 && header[4] != 32))									// Is The TGA 24 or 32 Bit?
	{
		fclose(file);														// If Anything Failed, Close The File
		return FALSE;														// Return False
	}

	texture->bpp = header[4];											// Grab The TGA's Bits Per Pixel (24 or 32)
	bytesPerPixel = texture->bpp / 8;										// Divide By 8 To Get The Bytes Per Pixel
	imageSize = texture->width*texture->height*bytesPerPixel;			// Calculate The Memory Required For The TGA Data

	texture->imageData = (GLubyte *)malloc(imageSize);						// Reserve Memory To Hold The TGA Data

	if (texture->imageData == NULL ||											// Does The Storage Memory Exist?
		fread(texture->imageData, 1, imageSize, file) != imageSize)			// Does The Image Size Match The Memory Reserved?
	{
		if (texture->imageData != NULL)										// Was Image Data Loaded
			free(texture->imageData);										// If So, Release The Image Data

		fclose(file);														// Close The File
		return FALSE;														// Return False
	}

	for (GLuint i = 0; i<int(imageSize); i += bytesPerPixel)						// Loop Through The Image Data
	{																		// Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
		temp = texture->imageData[i];											// Temporarily Store The Value At Image Data 'i'
		texture->imageData[i] = texture->imageData[i + 2];					// Set The 1st Byte To The Value Of The 3rd Byte
		texture->imageData[i + 2] = temp;									// Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
	}

	fclose(file);															// Close The File

																			// Build A Texture From The Data
	glGenTextures(1, &texture[0].texID);									// Generate OpenGL texture IDs
	glBindTexture(GL_TEXTURE_2D, texture[0].texID);							// Bind Our Texture
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);		// Linear Filtered
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);		// Linear Filtered
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	if (texture[0].bpp == 24)													// Was The TGA 24 Bits
	{
		type = GL_RGB;														// If So Set The 'type' To GL_RGB
	}

	glTexImage2D(GL_TEXTURE_2D, 0, type, texture[0].width, texture[0].height, 0, type, GL_UNSIGNED_BYTE, texture[0].imageData);

	return true;															// Texture Building Went Ok, Return True
}

// �ؽ�Ʈ ���Ͽ��� ��� ���� �о�鿩�� ��� �𵨸��� �����ϴ� �Լ�
void setup_world()
{
	float x, y, z, u, v;
	int numpolygons; // ������ �� ����
	int texid; // �ؽ��� ���̵�
	FILE *filein;
	char oneline[255]; // ��ü ������ �о�� ����
	filein = fopen("Data/world.txt", "rt");		// data/world.txt��� ������ ����.

	readstr(filein, oneline); // �� ������ �о�´�.
	sscanf(oneline, "NUMPOLLIES %d\n", &numpolygons); // ù ������ ������ �� ������ �����س��� ����, ���⼭ ������ �� ���� ���� ������ ���� ������ ����
	g_sector1.polygon = new POLYGON[numpolygons]; // ������ �� ������ŭ �����Ҵ�
	g_sector1.texture = new int[numpolygons];	 // ������ �� ������ŭ �����Ҵ�
	g_sector1.numpolygons = numpolygons; // ���������� �ִ� ��(������ �� ����)�� �ٽ� ���� ������ ����								

										 // ������ �� ������ŭ �ݺ�
	for (int loop = 0; loop < numpolygons; loop++)
	{
		readstr(filein, oneline); // �� ������ �о�´�.
		sscanf(oneline, "TEXTURE %d\n", &texid);	// ���������� �ؽ��� ���̵� ����
		g_sector1.texture[loop] = texid; // ����� �ؽ��� ���̵� ������ �ٽ� ����
		for (int vert = 0; vert < 4; vert++)
		{
			readstr(filein, oneline);	// �� ������ �о�´�
			sscanf(oneline, "%f %f %f %f %f %f %f", &x, &y, &z, &u, &v); // ����� ������� x, y, z, u, v�� ������� ������ ���� ������ ����
			g_sector1.polygon[loop].vertex[vert].x = x;
			g_sector1.polygon[loop].vertex[vert].y = y;
			g_sector1.polygon[loop].vertex[vert].z = z;
			g_sector1.polygon[loop].vertex[vert].u = u;
			g_sector1.polygon[loop].vertex[vert].v = v; // �ٽ� ���������� ����� ���� �ٽ� ���� ������ ����
		}
	}
	fclose(filein); // ������ ������ �ݴ´�.
	return;
}

// �ؽ�Ʈ ���Ͽ��� �� ���ξ� �о���� ���� �Լ�
void readstr(FILE* f, char* string)
{
	do
	{
		fgets(string, 255, f);
	} while ((string[0] == '/') || (string[0] == '\n'));
}

// �ؽ��� ���� �Լ�
bool setup_textures()
{
	// ��� �ϳ��� �ε��� �������� ���
	if (!load_TGA(&textures[0], "Data/floor.tga")
		|| !load_TGA(&textures[1], "Data/tiled.tga")
		|| !load_TGA(&textures[2], "Data/painting1.tga")
		|| !load_TGA(&textures[3], "Data/painting2.tga")
		|| !load_TGA(&textures[4], "Data/painting3.tga")
		|| !load_TGA(&textures[5], "Data/painting4.tga")
		|| !load_TGA(&textures[7], "Data/painting6.tga")
		|| !load_TGA(&textures[6], "Data/painting5.tga")
		|| !load_TGA(&textures[8], "Data/painting8.tga")
		|| !load_TGA(&textures[9], "Data/painting8.tga")
		|| !load_TGA(&textures[10], "Data/painting9.tga")
		|| !load_TGA(&textures[11], "Data/graywall.tga") // ȸ�� ��
		|| !load_TGA(&textures[12], "Data/door.tga")
		|| !load_TGA(&textures[13], "Data/crosshairs.tga")
		|| !load_TGA(&textures[14], "Data/hardwood.tga") // ����
		|| !load_TGA(&textures[15], "Data/stairsbottom.tga") // 
		|| !load_TGA(&textures[16], "Data/atticceiling.tga")
		|| !load_TGA(&textures[17], "Data/elevatordoor.tga") // ���������� ��
		|| !load_TGA(&textures[18], "Data/railing.tga")
		|| !load_TGA(&textures[19], "Data/font.tga")
		|| !load_TGA(&textures[20], "Data/opendoor.tga") // ��Ʋ
		|| !load_TGA(&textures[21], "Data/simpledoor.tga") // ��
		|| !load_TGA(&textures[22], "Data/column.tga")
		|| !load_TGA(&textures[23], "Data/screen.tga")
		|| !load_TGA(&textures[24], "Data/whiteboard.tga")
		|| !load_TGA(&textures[25], "Data/monitor.tga")
		|| !load_TGA(&textures[26], "Data/mirror.tga"))
		return false; // ���������� ����
	return true; // ��� �ؽ��� ������ �ùٸ��� �ε�Ǿ��ٸ� ���������� ����
}

// ��Ʈ �ؽ��� ����
GLvoid build_front(GLvoid)
{
	base = glGenLists(95);											// Creating 95 Display Lists
	glBindTexture(GL_TEXTURE_2D, textures[19].texID);				// Bind Our Font Texture
	for (int loop = 0; loop<95; loop++)								// Loop Through All 95 Lists
	{
		float cx = float(loop % 16) / 16.0f;								// X Position Of Current Character
		float cy = float(loop / 16) / 8.0f;								// Y Position Of Current Character

		glNewList(base + loop, GL_COMPILE);							// Start Building A List
		glBegin(GL_QUADS);											// Use A Quad For Each Character
		glTexCoord2f(cx, 1.0f - cy - 0.120f); glVertex2i(0, 0);	// Texture / Vertex Coord (Bottom Left)
		glTexCoord2f(cx + 0.0625f, 1.0f - cy - 0.120f); glVertex2i(16, 0);	// Texutre / Vertex Coord (Bottom Right)
		glTexCoord2f(cx + 0.0625f, 1.0f - cy);		  glVertex2i(16, 16);// Texture / Vertex Coord (Top Right)
		glTexCoord2f(cx, 1.0f - cy);		  glVertex2i(0, 16);	// Texture / Vertex Coord (Top Left)
		glEnd();													// Done Building Our Quad (Character)
		glTranslated(15, 0, 0);										// Move To The Right Of The Character
		glEndList();												// Done Building The Display List
	}																// Loop Until All 256 Are Built
}

// �ʱ�ȭ
bool init()
{
	glEnable(GL_LIGHT0);											// 0�� ���� Ȱ��ȭ
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, GlobalAmbient);			// ���� ���� �� ����

	setup_textures();												// �ؽ��� ����
	build_front();													// ��Ʈ �ؽ��� ����
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);							// Pixel Storage Mode To Byte Alignment
	glEnable(GL_TEXTURE_2D);										// �ؽ��� ���� Ȱ��ȭ
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);							// ���� �ʱ�ȭ
	glClearDepth(1.0f);												// ���� ����
	glDepthFunc(GL_LESS);											// ���� ���� ó�� ����
	glEnable(GL_DEPTH_TEST);										// ���� ���� Ȱ��ȭ
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);				// Alpha blending ����
	glEnable(GL_BLEND);												// blending Ȱ��ȭ
	glAlphaFunc(GL_GREATER, 0.1f);
	glEnable(GL_ALPHA_TEST);
	glShadeModel(GL_SMOOTH);										// ���� ���� �� ���
	setup_world();													// �ؽ�Ʈ ���Ͽ��� ��� ���� �о�鿩�� ��� �𵨸��� �����Ѵ�.
	memset(g_key, 0, sizeof(g_key));
	return true;
}

// ���÷��� �籸�� �̺�Ʈ
void reshape(int w, int h)
{
	window_width = w;
	window_height = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);									// ������� ����
	glLoadIdentity();												// �׵���ķ� �ʱ�ȭ
	if (h == 0)
		h = 1;
	gluPerspective(45.0f, (float)w / (float)h, 0.1, 100.0);			// ���� ����
	glMatrixMode(GL_MODELVIEW);										// �𵨺���� ����
	glLoadIdentity();												// �׵���ķ� �ʱ�ȭ
}

// ��Ʈ ���
GLvoid gl_print(GLint x, GLint y, const char *string, ...)
{
	char text[256];	 // �Ѱܿ� string ���� �����ϴ� ����
	va_list	ap;		// Pointer To List Of Arguments

					// string ���� ���ٸ�
	if (string == NULL)
		return;	// �ƹ� �͵� ���� �ʴ´�.

	va_start(ap, string);
	vsprintf(text, string, ap);
	va_end(ap);

	// �о���� string ���� �ؽ��Ŀ� �����Ͽ�
	// ȭ�鿡 �ؽ��� ������ ��ü�ν� ����Ѵ�.
	glBindTexture(GL_TEXTURE_2D, textures[19].texID);
	glPushMatrix();
	glLoadIdentity();
	glTranslated(x, y, 0);
	glListBase(base - 32);
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
	glPopMatrix();
}

// �ؽ��� ������Ʈ�� �����ϴ� �Լ�
// world.txt���� �о�鿩�� vertex (x, y, z, u,v) ���� �̿��Ͽ� �ؽ��� ������Ʈ ����
void texture_object(int polygonNum)
{
	GLfloat x_m, y_m, z_m, u_m, v_m;

	glBindTexture(GL_TEXTURE_2D, textures[g_sector1.texture[polygonNum]].texID);
	glBegin(GL_POLYGON);
	glNormal3f(0.0f, 0.0f, 1.0f); // �������� ���
	x_m = g_sector1.polygon[polygonNum].vertex[0].x;
	y_m = g_sector1.polygon[polygonNum].vertex[0].y;
	z_m = g_sector1.polygon[polygonNum].vertex[0].z;
	u_m = g_sector1.polygon[polygonNum].vertex[0].u;
	v_m = g_sector1.polygon[polygonNum].vertex[0].v;
	glTexCoord2f(u_m, v_m); glVertex3f(x_m, y_m, z_m);

	x_m = g_sector1.polygon[polygonNum].vertex[1].x;
	y_m = g_sector1.polygon[polygonNum].vertex[1].y;
	z_m = g_sector1.polygon[polygonNum].vertex[1].z;
	u_m = g_sector1.polygon[polygonNum].vertex[1].u;
	v_m = g_sector1.polygon[polygonNum].vertex[1].v;
	glTexCoord2f(u_m, v_m); glVertex3f(x_m, y_m, z_m);

	x_m = g_sector1.polygon[polygonNum].vertex[2].x;
	y_m = g_sector1.polygon[polygonNum].vertex[2].y;
	z_m = g_sector1.polygon[polygonNum].vertex[2].z;
	u_m = g_sector1.polygon[polygonNum].vertex[2].u;
	v_m = g_sector1.polygon[polygonNum].vertex[2].v;
	glTexCoord2f(u_m, v_m); glVertex3f(x_m, y_m, z_m);

	x_m = g_sector1.polygon[polygonNum].vertex[3].x;
	y_m = g_sector1.polygon[polygonNum].vertex[3].y;
	z_m = g_sector1.polygon[polygonNum].vertex[3].z;
	u_m = g_sector1.polygon[polygonNum].vertex[3].u;
	v_m = g_sector1.polygon[polygonNum].vertex[3].v;
	glTexCoord2f(u_m, v_m); glVertex3f(x_m, y_m, z_m);
	glEnd();
}

// ���÷��� �̺�Ʈ
void render(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	 // ���� ���� �� ���� ���� �ʱ�ȭ
	glLoadIdentity();	// �𵨺� ��� �ʱ�ȭ

	GLfloat xtrans = -g_xpos;
	GLfloat ztrans = -g_zpos;
	GLfloat ytrans = -g_ypos;
	if (g_yrot > 360)
		g_yrot -= 360;
	else if (g_yrot < 0)
		g_yrot += 360;
	GLfloat sceneroty = (360.0f - g_yrot);

	// 0�� ������ �ٶ󺸴� ����
	GLfloat round_x = sin(g_yrot * PI_OVER_180 / 180);
	GLfloat round_z = -cos(g_yrot * PI_OVER_180 / 180);

	GLfloat Light0_Ambient[] = { 1.0, 1.0, 1.0, 1.0 }; //0�� ���� Ư��
	GLfloat Light0_Diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat Light0_Specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat Light0_Position[] = { g_xpos, g_ypos, g_zpos + 1, 1.0 }; //0�� ���� ��ġ
	GLfloat Light0_Direction[] = { round_x, 0.0, round_z, 0.0 }; //0�� ���� ����
	GLfloat Light0_Cutoff[] = { 80.0 }; //0�� ���� ����

	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.5);
	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 1.0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, Light0_Ambient); //0�� ���� Ư���Ҵ�
	glLightfv(GL_LIGHT0, GL_DIFFUSE, Light0_Diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, Light0_Specular);
	glLightfv(GL_LIGHT0, GL_POSITION, Light0_Position); // 0�� ���� ��ġ �Ҵ�
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, Light0_Direction); // ����Ʈ����Ʈ ���� �Ҵ�
	glLightfv(GL_LIGHT0, GL_SPOT_CUTOFF, Light0_Cutoff); // ����Ʈ����Ʈ ���� �Ҵ�

	int numpolygons;

	glRotatef(g_lookupdown, 1.0f, 0, 0);
	glRotatef(sceneroty, 0, 1.0f, 0);

	glTranslatef(xtrans, ytrans, ztrans);

	numpolygons = g_sector1.numpolygons; // �������� �� ������ �����´�.

										 // �������� �� ������ŭ �ݺ�
	for (int loop_m = 0; loop_m < numpolygons; loop_m++)
		texture_object(loop_m);	// �ؽ��� ������Ʈ�� �����Ѵ�.

	glMatrixMode(GL_PROJECTION); // ������� ����					
	glPushMatrix();	// ������Ŀ� ���� ������İ� ����				
	glLoadIdentity(); // ������� �ʱ�ȭ								
	glOrtho(-10, window_width, 0, window_height, -10, 10);// ���� ���� �ǽ�

	glMatrixMode(GL_MODELVIEW);	// �𵨺� ��� ����

								// ȭ��� X,Y,Z ��ǥ�� ���
	glDisable(GL_LIGHTING);
	glEnable(GL_LIGHTING);

	// ENTER Ű�� ���� ���¶��
	// Ű����� �Է��� �� �ִ� Ű���� ��ɵ��� �˷��ش� 
	if (ENTER_WAS_PRESSED)
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		gl_print((window_width / 2) - 200, (window_height / 2) + 80, "Up Arrow: Move Forward");
		gl_print((window_width / 2) - 200, (window_height / 2) + 60, "Down Arrow: Move Backwards");
		gl_print((window_width / 2) - 200, (window_height / 2) + 40, "Left Arrow: Turn Left");
		gl_print((window_width / 2) - 200, (window_height / 2) + 20, "Right Arrow: Turn Right");
		gl_print((window_width / 2) - 200, (window_height / 2), "Page Up: Look up");
		gl_print((window_width / 2) - 200, (window_height / 2) - 20, "Page Down: Look Down");
		gl_print((window_width / 2) - 200, (window_height / 2) - 40, "Spacebar: Jump");
		gl_print((window_width / 2) - 200, (window_height / 2) - 60, "Plus Sign: Speed Up Movement");
		gl_print((window_width / 2) - 200, (window_height / 2) - 80, "Minus Sign: Slow Down Movement");
		gl_print((window_width / 2) - 200, (window_height / 2) - 100, "Mouse Left button : Light Side");
		gl_print((window_width / 2) - 200, (window_height / 2) - 120, "Mouse Right button : Light Off");
		glEnable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
	}

	// ���� �� �� �ִ� ���� ��ġ�� �����ٸ�
	if (g_zpos < 0.0 && g_zpos > -3.3 && g_xpos < 3.0 && g_xpos > 2.0 && g_ypos < 0.5) {
		glDisable(GL_LIGHTING);
		glEnable(GL_LIGHTING);
	}


	glMatrixMode(GL_PROJECTION); // ������� ����
	glPopMatrix(); // ����� ��İ� ����
	glMatrixMode(GL_MODELVIEW);	// �𵨺���� ����

								// ���� Ȱ��ȭ ���°��� ����
								// ���� Ȱ��ȭ �Ǵ� ��Ȱ��ȭ
	if (LIGHT_ON == true)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);

	glutSwapBuffers(); // ���
}

// ���� ���� �Լ�
// Ű���� �̺�Ʈ���� ���
// ���� �����ϴ� ��ġ���� �̺�Ʈ ��� ����
void openDoor()
{
	if (!DOOR_IS_OPEN) // ���� ���°� �������� ���� ���¶��
	{
		DOOR_IS_OPEN = true; // ���� ���¸� �� ���·� �ٲٰ�
							 // 110�� �ݺ��Ͽ� ���� ������ �ִϸ��̼��� �����.
		for (int i = 0; i < 110; i++)
		{
			g_sector1.polygon[0].vertex[0].x = 2.13 + (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[0].vertex[1].x = 2.13 + (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[0].vertex[0].z = -1.999 - (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[0].vertex[1].z = -1.999 - (0.373 - (0.373 * cos(i*PI_OVER_180)));
			Sleep(7); // ���� ������ ���� �������� ���ϵ��� ������ �Ǵ�.
			render(); // ���÷��� �̺�Ʈ ȣ��
		}
	}
	else // ���� ���°� �����ִ� ���¶��
	{
		DOOR_IS_OPEN = false; // ���� ���¸� �������� ���� ���·� �ٲٰ�
							  // 110�� ���÷��� �̺�Ʈ�� ȣ���Ͽ� ���� ������ �ִϸ��̼��� �����.
		for (int i = 0; i < 110; i++)
		{
			g_sector1.polygon[0].vertex[0].x = 2.61 - (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[0].vertex[1].x = 2.61 - (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[0].vertex[0].z = -2.49 + (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[0].vertex[1].z = -2.49 + (0.373 - (0.373 * cos(i*PI_OVER_180)));
			Sleep(7); // ���� ������ ���� �������� ���ϵ��� ������ �Ǵ�.
			render(); // ���÷��� �̺�Ʈ ȣ��
		}
	}
}

void openDoor_1()
{
	if (!DOOR_IS_OPEN) // ���� ���°� �������� ���� ���¶��
	{
		DOOR_IS_OPEN = true; // ���� ���¸� �� ���·� �ٲٰ�
							 // 110�� �ݺ��Ͽ� ���� ������ �ִϸ��̼��� �����.
		for (int i = 0; i < 110; i++)
		{
			g_sector1.polygon[0].vertex[0].x = 8.29 + (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[0].vertex[1].x = 8.29 + (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[0].vertex[0].z = -2.99 - (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[0].vertex[1].z = -2.99 - (0.373 - (0.373 * cos(i*PI_OVER_180)));
			Sleep(7); // ���� ������ ���� �������� ���ϵ��� ������ �Ǵ�.
			render(); // ���÷��� �̺�Ʈ ȣ��
		}
	}
	else // ���� ���°� �����ִ� ���¶��
	{
		DOOR_IS_OPEN = false; // ���� ���¸� �������� ���� ���·� �ٲٰ�
							  // 110�� ���÷��� �̺�Ʈ�� ȣ���Ͽ� ���� ������ �ִϸ��̼��� �����.
		for (int i = 0; i < 110; i++)
		{
			g_sector1.polygon[0].vertex[0].x = 8.77 - (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[0].vertex[1].x = 8.77 - (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[0].vertex[0].z = -3.481 + (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[0].vertex[1].z = -3.481 + (0.373 - (0.373 * cos(i*PI_OVER_180)));
			Sleep(7); // ���� ������ ���� �������� ���ϵ��� ������ �Ǵ�.
			render(); // ���÷��� �̺�Ʈ ȣ��
		}
	}
}

// Ű���� �̺�Ʈ
void keyboard(unsigned char key, int x, int y)
{
	GLfloat jumpTop = g_ypos + 0.25; // ������ �ö� �� �ִ� �ִ� y��
	GLfloat jumpBottom = g_ypos + (speed_XYZ / 2); // ���� �� ������ �� �ִ� �ּ� y��
	switch (key)
	{
	case ' ': // �����̽� Ű�� ���ȴٸ� ����
		if (g_zpos < -4.5 && g_xpos < 0.2 && g_xpos > -0.2)
			jumpBottom = 0.45 + (speed_XYZ / 2);
		g_ypos += (speed_XYZ / 4);
		while (g_ypos < jumpTop) // ���� top���� y���� ������
		{
			g_ypos += (speed_XYZ / 2); // y ���� ��� ����
			glutPostRedisplay(); // ���÷��� �̺�Ʈ ���� ȣ��
		}
		// ���� �ִ���� �ö�� ����
		while (g_ypos > jumpBottom) // ���� bottom���� y���� ũ�� 
		{
			g_ypos -= (speed_XYZ / 2); // y ���� ��� ����
			glutPostRedisplay(); // ���÷��� �̺�Ʈ ���� ȣ��
		}
		g_ypos -= (speed_XYZ / 4);
		// �ٽ� ���� y������ ����
		break;
	case 'x': // x �Ǵ�
	case 'X': // X�� �ԷµǾ��� ���
			  // ���� �ִ� ��ġ���
		if (g_zpos < 0.0 && g_zpos > -3.3 && g_xpos < 3.0 && g_xpos > 2.0 && g_ypos < 0.5) {
			openDoor(); // ���� ���ų� �ݴ´�.
		}
		else if (g_zpos < -2.0 && g_zpos > -4.0 && g_xpos < 9.0 && g_xpos > 8.0 && g_ypos < 0.5) {
			openDoor_1();
		}
		break;
	case '-': // -�� �ԷµǸ�
			  // �̵��ӵ��� ����
		if (speed_UDLR > 0.02 && speed_XYZ > 0.002)
		{
			speed_UDLR -= 0.01;
			speed_XYZ -= 0.001;
		}
		break;
	case '+': // +�� �ԷµǸ�
			  // �̵��ӵ��� ����
		speed_UDLR += 0.01;
		speed_XYZ += 0.001;
		break;
	case 13: // ENTER�� ���ȴٸ�
			 // ���� ���°� �ƴ϶��
		if (!ENTER_WAS_PRESSED)
			ENTER_WAS_PRESSED = true; // ������ ���� ���·� ��ȭ
		else // ���� ���°� �´ٸ�
			ENTER_WAS_PRESSED = false; // ������ ������ ���� ���·� ��ȭ
		break;
	case 27: // esc�� ���ȴٸ�
		exit(0); // �ý��� ����
		break;
	default:
		break;
	}
	render();
}

// Ư��Ű �̺�Ʈ
// Ű�� ���ȴٸ� Ű�� ���°��� true, ������ ���·� �Ѵ�.
void special_keys(int a_keys, int x, int y)
{
	switch (a_keys)
	{
	case GLUT_KEY_F1:
		if (!g_gamemode)
		{
			g_fullscreen = !g_fullscreen;						// Toggle g_fullscreen Flag
			if (g_fullscreen) glutFullScreen();					// We Went In Fullscreen Mode
			else glutReshapeWindow(window_width, window_height);  // We Went In Windowed Mode
		}
		break;
	default:
		g_key[a_keys] = true;
		break;
	}
}

// Ư��Ű�� ���� ���¿��� �������� ���� ���·� ��ȭ���� �� ȣ��Ǵ� �̺�Ʈ
void special_keys_up(int key, int x, int y)
{
	g_key[key] = false; // �� Ű�� ���� ���� ���� false, �� �������� ���� ���·� �Ѵ�.
}

// idle �̺�Ʈ�� ��ϵ� game_function
// ���� Ű�� ���°��� ���� ���������� �����Ͽ�
// ĳ������ �������� �����Ѵ�.
void game_function()
{
	// page up�� ���ȴٸ�
	// ������ ���� �����Ѵ�.
	if (g_key[GLUT_KEY_PAGE_UP])
	{
		g_z -= 0.002f;
		g_lookupdown -= speed_UDLR;
	}
	// page down�� ���ȴٸ�
	// ������ �Ʒ��� �Ѵ�..
	if (g_key[GLUT_KEY_PAGE_DOWN])
	{
		g_z += 0.002f;
		g_lookupdown += speed_UDLR;
	}
	// ���� ����Ű�� ���ȴٸ�
	// �����Ѵ�.
	if (g_key[GLUT_KEY_UP])
	{
		// �����ϴ� ���� ��� �κ��̶��
		// ĳ������ y�� ���� ��ܿ� ���缭 �����ϰų� ������Ų��. ��, �������������Ѵ�.
		if (g_zpos > 0.0 && g_zpos < 0.32 && g_xpos > 3.3 && g_xpos < 3.6 && g_ypos < 0.47 && g_ypos >= -0.27)
		{
			g_xpos -= (float)sin(g_yrot*PI_OVER_180) * (speed_XYZ / 2);
			g_zpos -= (float)cos(g_yrot*PI_OVER_180) * (speed_XYZ / 2);
			if (g_ypos > -0.27 && (g_yrot <= 200 && g_yrot >= 160))
				g_ypos -= ((3 * speed_XYZ / 5) + 0.006);
		}
		else if (g_zpos > 0.0 && g_zpos < 0.7 && g_xpos > 3.3 && g_xpos < 3.6 && g_ypos < 0.45 && g_ypos >= -0.57)
		{
			g_xpos -= (float)sin(g_yrot*PI_OVER_180) * (speed_XYZ / 2);
			g_zpos -= (float)cos(g_yrot*PI_OVER_180) * (speed_XYZ / 2);
			if (g_ypos > -0.57 && (g_yrot >= 270 || g_yrot <= 90))
				g_ypos += ((3 * speed_XYZ / 5) + 0.006);
		}
		// �׷��� �ʴٸ�
		// ĳ���Ͱ� �ٶ󺸴� �������� �����Ѵ�.
		else {
			g_xpos -= (float)sin(g_yrot*PI_OVER_180) * speed_XYZ;
			g_zpos -= (float)cos(g_yrot*PI_OVER_180) * speed_XYZ;

			// �浹�� �����Ѵٸ�
			// ������ ���´�.
			if (!no_collision())
			{
				g_xpos += (float)sin(g_yrot*PI_OVER_180) * speed_XYZ;
				g_zpos += (float)cos(g_yrot*PI_OVER_180) * speed_XYZ;
			}
		}
	}
	// �Ʒ��� ����Ű�� ���ȴٸ�
	// �����Ѵ�.
	if (g_key[GLUT_KEY_DOWN])
	{
		// �����ϴ� ���� ��� �κ��̶��
		// ĳ������ y�� ���� ��ܿ� ���缭 �����ϰų� ������Ų��. ��, �������������Ѵ�.
		if (g_zpos > 0.0 && g_zpos < 0.32 && g_xpos > 3.3 && g_xpos < 3.6 && g_ypos < 0.47 && g_ypos >= -0.27)
		{
			g_xpos -= (float)sin(g_yrot*PI_OVER_180) * (speed_XYZ / 2);
			g_zpos -= (float)cos(g_yrot*PI_OVER_180) * (speed_XYZ / 2);
			if (g_ypos > -0.27 && (g_yrot <= 200 && g_yrot >= 160))
				g_ypos -= ((3 * speed_XYZ / 5) + 0.006);
		}
		else if (g_zpos > 0.0 && g_zpos < 0.7 && g_xpos > 3.3 && g_xpos < 3.6 && g_ypos < 0.45 && g_ypos >= -0.57)
		{
			g_xpos -= (float)sin(g_yrot*PI_OVER_180) * (speed_XYZ / 2);
			g_zpos -= (float)cos(g_yrot*PI_OVER_180) * (speed_XYZ / 2);
			if (g_ypos > -0.57 && (g_yrot >= 270 || g_yrot <= 90))
				g_ypos += ((3 * speed_XYZ / 5) + 0.006);
		}
		else if (g_zpos > -0.1 && g_zpos < 0.32 && g_xpos > 3.8 && g_xpos < 4.4 && g_ypos < -0.15 && g_ypos >= -0.67)
		{
			g_xpos += (float)sin(g_yrot*PI_OVER_180) * (speed_XYZ / 2);
			g_zpos += (float)cos(g_yrot*PI_OVER_180) * (speed_XYZ / 2);
			if (g_ypos > -0.97 && (g_yrot >= 270 || g_yrot <= 90))
				g_ypos -= ((3 * speed_XYZ / 5) + 0.006);
		}
		else if (g_zpos > -0.1 && g_zpos < 0.7 && g_xpos > 3.8 && g_xpos < 4.4 && g_ypos < -0.15 && g_ypos >= -0.97)
		{
			g_xpos += (float)sin(g_yrot*PI_OVER_180) * (speed_XYZ / 2);
			g_zpos += (float)cos(g_yrot*PI_OVER_180) * (speed_XYZ / 2);
			if (g_ypos > -0.97 && (g_yrot <= 200 || g_yrot >= 160))
				g_ypos += ((3 * speed_XYZ / 5) + 0.006);
		}
		// �׷��� �ʴٸ�
		// ĳ���Ͱ� �ٶ󺸴� �ݴ� �������� �����Ѵ�.
		else
		{
			g_xpos += (float)sin(g_yrot*PI_OVER_180) * speed_XYZ;
			g_zpos += (float)cos(g_yrot*PI_OVER_180) * speed_XYZ;

			// �浹�� �����Ѵٸ�
			// ������ ���´�.
			if (!no_collision())
			{
				g_xpos -= (float)sin(g_yrot*PI_OVER_180) * speed_XYZ;
				g_zpos -= (float)cos(g_yrot*PI_OVER_180) * speed_XYZ;
			}
		}
	}
	// ������ ����Ű�� ���ȴٸ�
	// ������ ��� �����Ѵ�.
	if (g_key[GLUT_KEY_RIGHT])
		g_yrot -= speed_UDLR;
	// ���� ����Ű�� ���ȴٸ�
	// ������ �·� �����Ѵ�.
	if (g_key[GLUT_KEY_LEFT])
		g_yrot += speed_UDLR;
	//Sleep(1);
	glutPostRedisplay(); // ���÷��� �̺�Ʈ ���� ȣ��
}

// �浹�� �����ϴ� �Լ�
bool no_collision()
{
	return true;
}

// ���콺 Ŭ�� �̺�Ʈ
// ���� ��ư�� ������ ���� Ȱ��ȭ
// ������ ��ư�� ������ ���� ��Ȱ��ȭ
void mouse_click(int button, int state, int x, int y)
{
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
		LIGHT_ON = true;
	else if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN))
		LIGHT_ON = false;
}

// ���� �Լ�
int main(int argc, char** argv)
{
	glutInit(&argc, argv);										// GLUT �ʱ�ȭ
	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE);
	if (g_gamemode)
	{
		glutGameModeString("640x480:16");						// 640x480 16bpp ���� ��� ����
		if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE))			// ���� ��尡 Ȱ��ȭ�ƴٸ�
			glutEnterGameMode();								// ���� ���� ����
		else g_gamemode = false;								// �׷��� �ʴٸ� ������ ���� ���α׷� ����
	}
	screen_width = glutGet(GLUT_SCREEN_WIDTH);
	screen_height = glutGet(GLUT_SCREEN_HEIGHT);
	window_width = screen_width / 1.4;
	window_height = screen_height / 1.4;

	if (!g_gamemode)
	{
		glutInitWindowSize(window_width, window_height);           // ������ ������ ����
		glutInitWindowPosition((screen_width - window_width) / 2, (screen_height - window_height) / 2); // ���α׷� �����찡 �߾ӿ� ��ġ�ϵ��� ����
		glutCreateWindow("�п��� 3��");					// ���α׷� ����
	}

	init();
	glutIgnoreKeyRepeat(true);									// Ű�� �ݺ��� �����Ѵ�.
	glutDisplayFunc(render);									// ���÷��� �̺�Ʈ �ݹ� ���
	glutReshapeFunc(reshape);									// ���÷��� �籸�� �̺�Ʈ �ݹ� ���
	glutKeyboardFunc(keyboard);									// Ű���� �̺�Ʈ �ݹ� ���
	glutMouseFunc(mouse_click);									// ���콺 Ŭ�� �̺�Ʈ �ݹ� ���
	glutSpecialFunc(special_keys);								// Ư��Ű �̺�Ʈ �ݹ� ���
	glutSpecialUpFunc(special_keys_up);							// Ư��Ű�� ���� ���¿��� �������� ���� ���·� ��ȭ���� �� ȣ��Ǵ� �̺�Ʈ �ݹ� ���
	glutIdleFunc(game_function);								// idle �̺�Ʈ �ݹ� ���
	glutMainLoop();												// ���� ���� ����
	return 0;
}
