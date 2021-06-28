#include <iostream>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glut.h>    /* OpenGL Utility Toolkit header */

using namespace std;

// Định nghĩa màu sắc
#define GREY	0
#define RED	    1
#define GREEN	2
#define BLUE	3
#define CYAN	4
#define MAGENTA	5
#define YELLOW	6
#define BLACK	7

// Vật liệu màu sắc
static float materialColor[8][4] =
{
  {0.8, 0.8, 0.8, 1.0},
  {0.8, 0.0, 0.0, 1.0},
  {0.0, 0.5, 0.6, 1.0},
  {0.0, 0.0, 0.8, 1.0},
  {0.0, 0.8, 0.8, 1.0},
  {0.8, 0.0, 0.8, 1.0},
  {0.8, 0.8, 0.0, 1.0},
  {0.0, 0.0, 0.0, 0.6}
};

static int useRGB = 1; // khai báo biến sử dụng hệ màu red, green, blue
static int useLighting = 1; // khai báo biến sử dụng lighting (chiếu sáng)

// Hàm đặt màu cho đối tượng đối với việc sử dụng lighting hay màu RGB
static void setColor(int c)
{
    if (useLighting) {
        if (useRGB) {
            glMaterialfv(GL_FRONT_AND_BACK,
                GL_AMBIENT_AND_DIFFUSE, &materialColor[c][0]);
        }
        else {
            glMaterialfv(GL_FRONT_AND_BACK,
                GL_COLOR_INDEXES, &materialColor[c][0]);
        }
    }
    else {
        if (useRGB) {
            glColor4fv(&materialColor[c][0]);
        }
        else {
            glIndexf(materialColor[c][1]);
        }
    }
}

// Khai báo struct (hàm người dùng định nghĩa) định nghĩa cấu trúc Image (Ảnh)
struct Image
{
    unsigned long sizeX;
    unsigned long sizeY;
    char* data;
};

typedef struct Image Image;

// Hàm tải ảnh
int ImageLoad(const char* filename, Image* image)
{
    FILE* file;
    unsigned long size; // kích thước hình ảnh tính bằng byte
    unsigned long i; // biến đếm
    unsigned short int planes; // số mặt phẳng trong hình ảnh (phải là 1)
    unsigned short int bpp; // số bit trên mỗi pixel (phải là 24)
    char temp; // lưu trữ màu tạm thời để chuyển đổi bgr-rgb

    // mở file ảnh
    fopen_s(&file, filename, "rb");

    // tìm kiếm thông qua tiêu đề bmp
    fseek(file, 18, SEEK_CUR);

    // đọc chiều rộng
    if ((i = fread(&image->sizeX, 4, 1, file)) != 1) {
        return 0;
    }

    // đọc chiều dài
    if ((i = fread(&image->sizeY, 4, 1, file)) != 1) {
        return 0;
    }

    // tính toán kích thước (giả sử 24 bit hoặc 3 byte trên mỗi pixel)
    size = image->sizeX * image->sizeY * 3;

    // đọc số mặt phẳng
    if ((fread(&planes, 2, 1, file)) != 1) {
        return 0;
    }
    if (planes != 1) {
        return 0;
    }

    // đọc số bits trên 1 pixel
    if ((i = fread(&bpp, 2, 1, file)) != 1) {
        return 0;
    }
    if (bpp != 24) {
        return 0;
    }

    // tìm kiếm qua phần còn lại của tiêu đề bitmap
    fseek(file, 24, SEEK_CUR);

    // đọc dữ liệu
    image->data = (char*)malloc(size);
    if (image->data == NULL) {
        return 0;
    }
    if ((i = fread(image->data, size, 1, file)) != 1) {
        return 0;
    }

    // đảo ngược tất cả các màu (bgr -> rgb)
    for (i = 0; i < size; i += 3) {
        temp = image->data[i];
        image->data[i] = image->data[i + 2];
        image->data[i + 2] = temp;
    }
    return 1;
}

// Hàm dán ảnh
GLuint LoadBMP(const char* fileName)
{
    Image* image1;

    // phân bổ không gian cho kết cấu
    image1 = (Image*)malloc(sizeof(Image));

    // nếu không có ảnh, thoát chương trình
    if (image1 == NULL) {
        exit(0);
    }

    // nếu không load (tải) được ảnh, thoát chương trình
    if (!ImageLoad(fileName, image1)) {
        exit(1);
    }

    GLuint texture; // tạo một kết cấu OpenGL
    glGenTextures(1, &texture); // bắt đầu quá trình gen texture

    glBindTexture(GL_TEXTURE_2D, texture); // các chức năng của kết cấu trong tương lai sẽ sửa đổi kết cấu này
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // chia tỷ lệ tuyến tính khi hình ảnh lớn hơn kết cấu
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // chia tỷ lệ tuyến tính khi hình ảnh nhỏ hơn kết cấu

    // cung cấp hình ảnh cho OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->sizeX, image1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->data);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    return texture;
}

// Hàm vẽ văn bản 
void drawBitmapText(string text, int x, int y)
{
    glDisable(GL_LIGHTING);

    // lưu và đặt lại ma trận chế độ xem mô hình
    glPushMatrix();
    glLoadIdentity();

    // lưu và đặt lại ma trận chiếu
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    // nhận kích thước màn hình khung nhìn và chuyển sang chế độ chiếu trực quan 2D
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    gluOrtho2D(0, viewport[2], 0, viewport[3]);

    glColor3f(1, 1, 1);
    glRasterPos2f(x, y);
    // vẽ chữ cái theo phông chữ, cỡ chữ
    for (unsigned int i = 0; i < text.length(); i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);

    glPopMatrix();

    // đặt lại về ma trận chế độ xem mô hình đã lưu trước đó
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

// Hàm tạo ma trận chiếu bóng
void shadowMatrix(GLfloat shadowMat[4][4], GLfloat groundplane[4], GLfloat lightpos[4])
{
    GLfloat dot;

    // tìm tích điểm giữa vectơ vị trí ánh sáng và pháp tuyến mặt đất
    dot = groundplane[0] * lightpos[0] +
        groundplane[1] * lightpos[1] +
        groundplane[2] * lightpos[2] +
        groundplane[3] * lightpos[3];

    shadowMat[0][0] = dot - lightpos[0] * groundplane[0];
    shadowMat[1][0] = 0.f - lightpos[0] * groundplane[1];
    shadowMat[2][0] = 0.f - lightpos[0] * groundplane[2];
    shadowMat[3][0] = 0.f - lightpos[0] * groundplane[3];

    shadowMat[0][1] = 0.f - lightpos[1] * groundplane[0];
    shadowMat[1][1] = dot - lightpos[1] * groundplane[1];
    shadowMat[2][1] = 0.f - lightpos[1] * groundplane[2];
    shadowMat[3][1] = 0.f - lightpos[1] * groundplane[3];

    shadowMat[0][2] = 0.f - lightpos[2] * groundplane[0];
    shadowMat[1][2] = 0.f - lightpos[2] * groundplane[1];
    shadowMat[2][2] = dot - lightpos[2] * groundplane[2];
    shadowMat[3][2] = 0.f - lightpos[2] * groundplane[3];

    shadowMat[0][3] = 0.f - lightpos[3] * groundplane[0];
    shadowMat[1][3] = 0.f - lightpos[3] * groundplane[1];
    shadowMat[2][3] = 0.f - lightpos[3] * groundplane[2];
    shadowMat[3][3] = dot - lightpos[3] * groundplane[3];
}

// Hàm tìm phương trình mặt phẳng cho trước 3 điểm để vẽ bóng
void findPlane(GLfloat plane[4], GLfloat v0[3], GLfloat v1[3], GLfloat v2[3])
{
    GLfloat vec0[3], vec1[3];

    // cần 2 vectơ để tìm tích chéo
    vec0[0] = v1[0] - v0[0];
    vec0[1] = v1[1] - v0[1];
    vec0[2] = v1[2] - v0[2];

    vec1[0] = v2[0] - v0[0];
    vec1[1] = v2[1] - v0[1];
    vec1[2] = v2[2] - v0[2];

    // tìm tích chéo để có A, B và C của phương trình mặt phẳng
    plane[0] = vec0[1] * vec1[2] - vec0[2] * vec1[1];
    plane[1] = -(vec0[0] * vec1[2] - vec0[2] * vec1[0]);
    plane[2] = vec0[0] * vec1[1] - vec0[1] * vec1[0];

    plane[3] = -(plane[0] * v0[0] + plane[1] * v0[1] + plane[2] * v0[2]);
}

int angleX, angleY, angleZ = 0;
float spin = 0;
static GLuint texture[6];

// Hàm xử lý tốc độ quay
void spinDisplay() {
    spin = spin + 2.0; // xoay thêm độ cho mỗi lần lặp
    if (spin > 360)
        spin = spin - 360;
    glutPostRedisplay(); // thông báo cho chương trình thực hiện việc vẽ lại
}

double v, s, last_t = 0;
double start = 8;
double ground = -1.35;
bool firstTime = TRUE;
bool stop = TRUE;

// Hàm gieo xúc xắc
void Bounce(bool stop)
{
    if (stop == TRUE)
    {
        glutIdleFunc(spinDisplay); // khi chương trình đang trong trạng thái không phải xử lý gì, thực hiện hàm hàm xoay
        const double t = glutGet(GLUT_ELAPSED_TIME) / 800.0; // sử dụng hàm lấy thời gian để tính thời gian

        // khi chương trình được mở ra
        if (firstTime) {
            firstTime = FALSE;
            last_t = t; // thời gian cuối cùng
            s = start; // quãng đường xuất phát từ s
        }

        v += -9.8 * (t - last_t); // tính vận tốc = gia tốc x thời gian
        s += v * (t - last_t); // tính quãng đường = vận tốc x thời gian

        if (s >= start) {
            s = start; // giới hạn trên
            v = -0.8 * v; // nhân với 1 số âm để vận tốc giảm dần
        }
        if (s <= ground) {
            s = ground; // giới hạn dưới
            v = -0.8 * v;
        }
        last_t = t;

        // fabs : hàm trả về giá trị tuyệt đối
        if ((fabs(v) < 0.01) && (fabs(s - ground) < 0.01)) {
            glutIdleFunc(NULL);
        }
    }
    else {
        glutIdleFunc(NULL);
    }
}

// xác định tọa độ 6 mặt của 2 hình lập phương
float x = -1.0, y = -1, z = -1, xx = 1, yy = 1, zz = 1;

float face1[6][4][3] = { {{x, y, zz}, {xx, y, zz}, {xx, yy, zz}, {x, yy, zz}},  //front
    {{x, yy, z}, {xx, yy, z}, {xx, y, z}, {x, y, z}},   //back
    {{xx, y, zz}, {xx, y, z}, {xx, yy, z}, {xx, yy, zz}},   //right
    {{x, y, zz}, {x, yy, zz}, {x, yy, z}, {x, y, z}},   //left
    {{x, yy, zz}, {xx, yy, zz}, {xx, yy, z}, {x, yy, z}},   //top
    {{x, y, zz}, {x, y, z}, {xx, y, z}, {xx, y, zz}}    //bottom
};

float face2[6][4][3] = { {{x, yy, zz}, {xx, yy, zz}, {xx, yy, z}, {x, yy, z}},  //top
    {{x, y, zz}, {xx, y, zz}, {xx, yy, zz}, {x, yy, zz}},  //front
    {{x, y, zz}, {x, yy, zz}, {x, yy, z}, {x, y, z}},   //left
    {{x, y, zz}, {x, y, z}, {xx, y, z}, {xx, y, zz}},   //bottom
    {{x, yy, z}, {xx, yy, z}, {xx, y, z}, {x, y, z}},   //back
    {{xx, y, zz}, {xx, y, z}, {xx, yy, z}, {xx, yy, zz}}    //right
};

static int useTexture = 1;

// Hàm vẽ hình lập phương kèm theo màu sắc và chuyển động 3D
static void drawCube1() {
    setColor(RED);
    glTranslatef(0.0, 3.0 + s, 0.0);
    glTranslatef(4.0, 0.0, 0.0);
    glRotatef(spin, 1.0, 1.0, 1.0);
    glRotatef(angleX, 1, 0, 0);
    glRotatef(angleY, 0, 1, 0);
    glRotatef(angleZ, 0, 0, 1);

    for (int i = 0; i < 6; ++i) {
        glNormal3fv(face1[i][0]); // đặt vectơ pháp tuyến
        glBindTexture(GL_TEXTURE_2D, texture[i]);
        glBegin(GL_POLYGON);
        // hàm glTexcoord2f để map texture vào từng face của cube
        glTexCoord2f(0.0, 0.0); glVertex3fv(face1[i][0]);
        glTexCoord2f(1.0, 0.0); glVertex3fv(face1[i][1]);
        glTexCoord2f(1.0, 1.0); glVertex3fv(face1[i][2]);
        glTexCoord2f(0.0, 1.0); glVertex3fv(face1[i][3]);
        glEnd();
    }
}

static void drawCube2() {
    setColor(MAGENTA);
    glTranslatef(0.0, 3.0 + s, 0.0);
    glTranslatef(-4.0, 0.0, 0.0);
    glRotatef(spin, 1.0, 1.0, 1.0);
    glRotatef(angleX, 1, 0, 0);
    glRotatef(angleY, 0, 1, 0);
    glRotatef(angleZ, 0, 0, 1);

    for (int i = 0; i < 6; ++i) {
        //glNormal3fv(face2[i][0]);
        glBindTexture(GL_TEXTURE_2D, texture[i]);
        glBegin(GL_POLYGON);
        glTexCoord2f(0.0, 0.0); glVertex3fv(face2[i][0]);
        glTexCoord2f(1.0, 0.0); glVertex3fv(face2[i][1]);
        glTexCoord2f(1.0, 1.0); glVertex3fv(face2[i][2]);
        glTexCoord2f(0.0, 1.0); glVertex3fv(face2[i][3]);
        glEnd();
    }
}

// Hàm vẽ xúc xắc sử dụng texture hoặc lighting
static void drawDice() {
    glEnable(GL_LIGHTING);

    if (useTexture) {
        glEnable(GL_TEXTURE_2D);
    }

    glPushMatrix();
    drawCube1();
    glPopMatrix();

    glPushMatrix();
    drawCube2();
    glPopMatrix();

    if (useTexture) {
        glDisable(GL_TEXTURE_2D);
    }

    glDisable(GL_LIGHTING);
}

// Tọa độ sàn
static GLfloat floorVertices[4][3] = {
  { -10.0, 0.0, -10.0 },
  { -10.0, 0.0, 10.0 },
  { 10.0, 0.0, 10.0 },
  { 10.0, 0.0, -10.0 }
};

// Hàm vẽ sàn
static void drawFloor(void)
{
    //glDisable(GL_LIGHTING);
    glBegin(GL_QUADS);
    glVertex3fv(floorVertices[0]);
    glVertex3fv(floorVertices[1]);
    glVertex3fv(floorVertices[2]);
    glVertex3fv(floorVertices[3]);
    glEnd();
    //glEnable(GL_LIGHTING);
}

static GLfloat lightPosition[4];

// Hàm định vị nguồn sáng
static void Light() {
    lightPosition[0] = 0; //x
    lightPosition[1] = 15; //y
    lightPosition[2] = 0; //z
    lightPosition[3] = 1; // alpha
}

static GLfloat floorShadow[4][4];
static GLfloat floorPlane[4];

// Hàm vẽ bóng trên sàn
static void drawGroundShadow() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_LIGHTING);
    glColor4f(0.0, 0.0, 0.0, 0.5); //Tô 50 % màu đen cho đối tượng nằm trên sàn (bóng của vật thể)

    // vẽ bóng hlp 1
    glPushMatrix();
    glTranslatef(0, 0.005, 0);
    glMultMatrixf((GLfloat*)floorShadow); // hàm nhân ma trận hiện tại với một ma trận khác
    drawCube1();
    glPopMatrix();

    // vẽ bóng hlp 2
    glPushMatrix();
    glTranslatef(0, 0.005, 0);
    glMultMatrixf((GLfloat*)floorShadow);
    drawCube2();
    glPopMatrix();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

// Hàm vẽ hình ảnh phản chiếu trên sàn
static void drawGroundReflection() {
    /* Cập nhật các pixel có giá trị stencil là 1, để đảm bảo hình ảnh phản chiếu chỉ ở trên sàn, chứ không phải dưới sàn */

    // không cập nhật màu sắc hoặc độ sâu
    glDisable(GL_DEPTH_TEST);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    // vẽ 1 vào bộ đệm stencil
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 1, 0xffffffff);

    // vẽ sàn với pixel sàn chỉ cần đặt stencil của chúng thành 1
    glTranslatef(0.0, -1.5, 0.0);
    drawFloor();

    // bật lại cập nhật màu sắc và độ sâu
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    // bây giờ, chỉ hiển thị khi stencil được đặt thành 1
    glStencilFunc(GL_EQUAL, 1, 0xffffffff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glPushMatrix();

    // phản chiếu qua sàn (mặt phẳng Y = 0) để hạ xuống
    glScalef(1.0, -1.0, 1.0);

    // làm mờ ánh sáng trên phản chiếu, bật chế độ bình thường
    glEnable(GL_NORMALIZE);
    glCullFace(GL_FRONT);

    // vẽ xúc xắc phản chiếu
    drawDice();

    // tắt noramlize và bật lại tính năng back face culling
    glDisable(GL_NORMALIZE);
    glCullFace(GL_BACK);

    glPopMatrix();

    glDisable(GL_STENCIL_TEST);
}

GLfloat angle = 0;  // góc nhìn ban đầu theo trục y
GLfloat angle2 = 0; // góc nhìn ban đầu theo trục x

int a = 0, b = 10, c = 18; // khai báo điểm đặt mắt nhìn trong gluLookAt

static int renderReflection = 1; // khai báo biến sử dụng phản chiếu

// Hàm hiển thị
static void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // xóa mọi pixel
    glLoadIdentity(); // thay thế ma trận hiện tại bằng ma trận nhận dạng

    // thiết lập view
    gluLookAt(a, b, c, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); // (Tọa độ đặt mắt nhìn | vị trí trung tâm đặt tại tọa độ | trục hướng lên trên (trục y)

    drawBitmapText("Nhan chuot phai de mo Menu.", 50, 150);
    drawBitmapText("Nhan phim + de zoom in, phim - de zoom out.", 50, 125);
    drawBitmapText("Nhan phim up, down, left, right de thay doi goc nhin.", 50, 100);
    drawBitmapText("Nhan phim x de quay xuc xac nguoc chieu Ox, phim X de quay cung chieu Ox.", 50, 75);
    drawBitmapText("Nhan phim y de quay xuc xac nguoc chieu Oy, phim Y de quay cung chieu Oy.", 50, 50);
    drawBitmapText("Nhan phim z de quay xuc xac nguoc chieu Oz, phim Z de quay cung chieu Oz.", 50, 25);

    shadowMatrix(floorShadow, floorPlane, lightPosition);

    // thực hiện xoay cảnh dựa trên sự di chuột của người dùng
    glRotatef(angle2, 1.0, 0.0, 0.0);
    glRotatef(angle, 0.0, 1.0, 0.0);

    // cho GL biết vị trí nguồn sáng
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    if (renderReflection) {
        glPushMatrix();
        drawGroundReflection();     /* vẽ phản chiếu trên sàn */
        glPopMatrix();
    }

    glEnable(GL_BLEND);
    glColor4f(1.0, 1.0, 1.0, 0.3);
    glTranslatef(0.0, -1.5, 0.0);
    drawFloor();                    /* vẽ sàn */
    glDisable(GL_BLEND);

    Bounce(stop);
    glPushMatrix();
    drawDice();                     /* vẽ xúc xắc */
    glPopMatrix();

    if (!renderReflection) {
        glPushMatrix();
        drawGroundShadow();         /* vẽ bóng trên sàn */
        glPopMatrix();
    }

    glPushMatrix();
    Light();                        /* vẽ vị trí điểm chiếu sáng */
    glPopMatrix();

    glutSwapBuffers(); // thực hiện việc hoán đổi 2 buffer
}

int startx, starty;

static void mouse(int button, int state, int x, int y)
{
    startx = x;
    starty = y;
}

// Hàm xoay góc nhìn (di chuột trái)
static void motion(int x, int y)
{
    angle = angle + (x - startx);
    angle2 = angle2 + (y - starty);
    startx = x;
    starty = y;
    glutPostRedisplay();
}

// Hàm menu (nhấn chuột phải để mở)
static void menu_select(int mode)
{
    switch (mode) {
    case 1: // gieo xúc xắc
        stop = true;
        break;
    case 2: // dừng gieo
        stop = false;
        break;
    case 3: // tắt bật Texture (tắt Texture, đối tượng còn Lighting)
        useTexture = !useTexture;
        break;
    case 4: // chuyển đổi Reflection và Shadow
        renderReflection = !renderReflection;
        break;
    case 5: // thoát
        exit(0);
        break;
    }
    glutPostRedisplay();
}

// Các thao tác cần làm khi cửa sổ bị thay đổi kích thước
void reshape(int w, int h)
{
    // thiết lập các thông số cho view
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    // xét thao tác chiếu
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // chiếu phối cảnh
    gluPerspective(60.0, (GLfloat)w / (GLfloat)h, 1.0, 30.0); // góc nhìn | tỷ lệ khung hình | near | far

    // xét thao tác trên ModelView - khởi tạo ban đầu
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Hàm xử lý sự kiện keyboard
static void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        // xoay theo trục x, y, z 
    case 'x':
        angleX = (angleX + 10) % 360;
        break;
    case 'X':
        angleX = (angleX - 10) % 360;
        break;
    case 'y':
        angleY = (angleY + 10) % 360;
        break;
    case 'Y':
        angleY = (angleY - 10) % 360;
        break;
    case 'z':
        angleZ = (angleZ + 10) % 360;
        break;
    case 'Z':
        angleZ = (angleZ - 10) % 360;
        break;

        // zoom in, zoom out
    case '+':
        if (c > 12)
            c = c - 1;
        break;
    case '-':
        if (c < 22)
            c = c + 1;
        break;
    }
    stop = FALSE;
    glutPostRedisplay();
}

void keyboard2(int key, int x, int y)
{
    switch (key) {
        // thay đổi góc nhìn trái phải trên dưới
    case GLUT_KEY_UP:
        if (b > 0)
            b = b - 1;
        break;
    case GLUT_KEY_DOWN:
        if (b < 15)
            b = b + 1;
        break;
    case GLUT_KEY_LEFT:
        if (a > -10)
            a = a - 1;
        break;
    case GLUT_KEY_RIGHT:
        if (a < 10)
            a = a + 1;
        break;
    default: break;
    }
    stop = FALSE;
    glutPostRedisplay();
}

static float lightAmb[4] = { 0.2, 0.2, 0.2, 1.0 };
static float lightDiff[4] = { 0.8, 0.8, 0.8, 1.0 };
static float lightSpec[4] = { 0.4, 0.4, 0.4, 1.0 };

// hàm thực hiện các khởi tạo
void init(void)
{
    glClearColor(0.0, 0.5, 0.6, 1.0); // xác định màu để xóa color buffer (nền)
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST); // khử mặt khuất

    // lighting
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb); // ánh sáng môi trường
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiff); // ánh sáng khuếch tán
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec); // ánh sáng phản xạ
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_TEXTURE_2D);
    texture[0] = LoadBMP("1.bmp");
    texture[1] = LoadBMP("2.bmp");
    texture[2] = LoadBMP("3.bmp");
    texture[3] = LoadBMP("4.bmp");
    texture[4] = LoadBMP("5.bmp");
    texture[5] = LoadBMP("6.bmp");
    glDisable(GL_TEXTURE_2D);

    // thiết lập sơ đồ mặt phẳng để tính toán bóng chiếu dự kiến
    findPlane(floorPlane, floorVertices[1], floorVertices[2], floorVertices[3]);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); // khởi tạo chế độ vẽ single buffer và hệ màu RGB
    glutInitWindowSize(850, 650); // khởi tạo window kích thước window
    glutInitWindowPosition(300, 100); // khởi tạo window tại ví trí trên màn hình
    glutCreateWindow("3D Dice - Nhom 7"); // tên của window

    // đăng ký gọi GLUT
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(keyboard2);
    glutPostRedisplay();

    glutCreateMenu(menu_select);
    glutAddMenuEntry("Start motion", 1);
    glutAddMenuEntry("Stop", 2);
    glutAddMenuEntry("Toggle texture / lighting", 3);
    glutAddMenuEntry("Toggle shadow / refelction", 4);
    glutAddMenuEntry("Quit", 5);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    init(); // khởi tạo một số chế độ đồ họa

    glutMainLoop(); // bắt đầu chu trình lặp thể hiện vẽ
    return 0;
}
