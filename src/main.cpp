#include <iostream>
#include <vector>
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <dirent.h>
#include <algorithm>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <tgmath.h>

#define WIDTH 800
#define HEIGHT 600

typedef struct AppData{
    TTF_Font *font;
    SDL_Texture *phrase;
    SDL_Texture *folder;
    SDL_Texture *exe;
    SDL_Texture *image;
    SDL_Texture *video;
    SDL_Texture *code;
    SDL_Texture *other;
    SDL_Texture *next;
    SDL_Texture *back;
    SDL_Surface *phrase_surf;
    SDL_Color color;
    char *path;
    int focus;
}AppData;

void initialize(SDL_Renderer *renderer, AppData *data_ptr);
void render(SDL_Renderer *renderer, AppData *data_ptr);
void quit(AppData *data_ptr);
std::string getParent(std::string filepath);
bool pollevent(int *x, int *y);
int validRegions(int size, AppData* data_ptr);
int clicked(int x, int y, int regions);
std::string getType(std::string target, AppData* data_ptr);

int main(int argc, char **argv)
{
    char *home = getenv("HOME");
    printf("HOME: %s\n", home);


    // initializing SDL as Video
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    // create window and renderer
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer);

    // initialize and perform rendering loop
    AppData data;
    initialize(renderer, &data);
    render(renderer, &data);
    SDL_Event event;
    SDL_WaitEvent(&event);
    while (event.type != SDL_QUIT)
    {
        //render(renderer);
        //SDL_WaitEvent(&event);

        render(renderer, &data);
    }

    // clean up
    quit(&data);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}

void initialize(SDL_Renderer *renderer, AppData *data_ptr)
{
    //Fill in the data for our App_data struct
    data_ptr->font = TTF_OpenFont("resrc/OpenSans-Regular.ttf", 24);

    data_ptr->focus = 0;

    data_ptr->path = getenv("HOME");

    data_ptr->color ={0,0,0};

    SDL_Surface * img_surf = IMG_Load("resrc/images/folder.png");
    data_ptr->folder = SDL_CreateTextureFromSurface(renderer, img_surf);
    SDL_FreeSurface(img_surf);

    img_surf = IMG_Load("resrc/images/exe.png");
    data_ptr->exe = SDL_CreateTextureFromSurface(renderer, img_surf);
    SDL_FreeSurface(img_surf);

    img_surf = IMG_Load("resrc/images/code.png");
    data_ptr->code = SDL_CreateTextureFromSurface(renderer, img_surf);
    SDL_FreeSurface(img_surf);

    img_surf = IMG_Load("resrc/images/image.png");
    data_ptr->image = SDL_CreateTextureFromSurface(renderer, img_surf);
    SDL_FreeSurface(img_surf);

    img_surf = IMG_Load("resrc/images/other.png");
    data_ptr->other = SDL_CreateTextureFromSurface(renderer, img_surf);
    SDL_FreeSurface(img_surf);

    img_surf = IMG_Load("resrc/images/video.png");
    data_ptr->video = SDL_CreateTextureFromSurface(renderer, img_surf);
    SDL_FreeSurface(img_surf);

    img_surf = IMG_Load("resrc/images/next.png");
    data_ptr->next = SDL_CreateTextureFromSurface(renderer, img_surf);
    SDL_FreeSurface(img_surf);

    img_surf = IMG_Load("resrc/images/back.png");
    data_ptr->back = SDL_CreateTextureFromSurface(renderer, img_surf);
    SDL_FreeSurface(img_surf);

    data_ptr->phrase_surf = TTF_RenderText_Solid(data_ptr->font, "temp",data_ptr->color);
    
    data_ptr->phrase = SDL_CreateTextureFromSurface(renderer, data_ptr->phrase_surf);
    SDL_FreeSurface(data_ptr->phrase_surf); 
    
}

void render(SDL_Renderer *renderer, AppData *data_ptr) 
{
    // erase renderer content
    SDL_SetRenderDrawColor(renderer, 235, 235, 235, 255);
    SDL_RenderClear(renderer);
    
    // TODO: draw!
    //actually draw images/text(set size, position, ect.)
    SDL_Rect rect;

    std::vector<std::string> list;
    DIR *d;
    struct dirent *dir;
    d = opendir(data_ptr->path); 

    int y = 0;
    rect.w = 50;
    rect.h = 50;
    rect.x = 0;
    rect.y = y;
    SDL_Event e;
    int mouse_x =0;
    int mouse_y =0;
    bool click = false;
    std::string new_path;
    std::string temp = "";
    int regions;
    std::string prev = getParent(data_ptr->path);
    std::string type;
    int index=0;

    //put all items in the /home/<user> directory(including "..") into a vector
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            for(int i=0; i<strlen(dir->d_name); i++)
            {
                if(dir->d_name[i] != ' ')
                {
                    temp = temp + dir->d_name[i];
                }
            }
            if(temp == "..")
            {
                list.push_back(temp);
            }
            
            else if(dir->d_name[0] != '.')
            {
                list.push_back(dir->d_name);
                
            }
            temp = "";
            
        }
        closedir(d);

        //sort the list alphabetically
        std::sort(list.begin(), list.end());
    }
    //used to determine which parts of the window should be clickable
    //max of 8, correlated with files or directories
    regions = validRegions(list.size(), data_ptr);

    
    //determines what icon should be used for each item in a directory
    for (int i = 0; i < list.size() - (data_ptr->focus * 8); i++)
    {
        rect.y = y;
        index =i + (data_ptr->focus * 8);
        type = getType(list.at(index), data_ptr);

        if(type == "folder"){
            SDL_RenderCopy(renderer, data_ptr->folder, NULL, &rect);

        }else if (type == "exe")
        {
            SDL_RenderCopy(renderer, data_ptr->exe, NULL, &rect);
        }else if (type == "image")
        {
            SDL_RenderCopy(renderer, data_ptr->image, NULL, &rect);
        }else if (type == "video")
        {
            SDL_RenderCopy(renderer, data_ptr->video, NULL, &rect);
        }else if (type == "code")
        {
            SDL_RenderCopy(renderer, data_ptr->code, NULL, &rect);
        }else
        {
            SDL_RenderCopy(renderer, data_ptr->other, NULL, &rect);
        }
        y=y+75;
    }
    rect.x = 55;
    y = 0;
    index =0;

    //renders the appropriate text for each item in the directory
    for (int i = 0; i < list.size() - (data_ptr->focus * 8); i++)
    {
        index =i + (data_ptr->focus * 8);
        rect.y = y;
        data_ptr->phrase_surf = TTF_RenderText_Solid(data_ptr->font, list.at(index).c_str(),data_ptr->color);
        data_ptr->phrase = SDL_CreateTextureFromSurface(renderer, data_ptr->phrase_surf);
        SDL_QueryTexture(data_ptr->phrase, NULL, NULL, &(rect.w), &(rect.h));
        SDL_RenderCopy(renderer, data_ptr->phrase, NULL, &rect);
        SDL_FreeSurface(data_ptr->phrase_surf); 
        y=y+75;
    }

    rect.h = 100;
    rect.w = 100;
    rect.y = 500;

    rect.x = 600;
    SDL_RenderCopy(renderer, data_ptr->back, NULL, &rect);

    rect.x = 700;
    SDL_RenderCopy(renderer, data_ptr->next, NULL, &rect);




    // show rendered frame
    SDL_RenderPresent(renderer);

        //handles clicks on items in the window
        while(click==false)
    {
        click = pollevent(&mouse_x , &mouse_y);

        if(click)
        {
            if(clicked(mouse_x, mouse_y, regions) == -1)
            {

            }
            else if(clicked(mouse_x, mouse_y, regions) == 8)
            {
                if(data_ptr->focus > 0)
                {
                    data_ptr->focus = data_ptr->focus -1;
                }
            }
            else if(clicked(mouse_x, mouse_y, regions) == 9)
            {
                if (list.size() > 8 && data_ptr->focus < std::ceil(list.size()/8.0)-1)
                {
                    data_ptr->focus = data_ptr->focus + 1;
                }
                
            }
            else if(list.at(clicked(mouse_x, mouse_y, regions) + (data_ptr->focus * 8))=="..")
            {
                if(getParent(data_ptr->path) == "/")
                {
                    strcpy(data_ptr->path, "/");
                    data_ptr->focus =0;
                }
                else if(getParent(data_ptr->path) == "")
                {

                }
                else
                {
                    strcpy(data_ptr->path,prev.c_str());
                    data_ptr->focus =0;
                    std::cout << data_ptr->path << '\n';
                }
            }
            else
            {
                new_path = "/" + list.at(clicked(mouse_x, mouse_y, regions) + (data_ptr->focus * 8));
                new_path = data_ptr->path + new_path;
                strcpy(data_ptr->path,new_path.c_str());
                data_ptr->focus =0;
                std::cout << data_ptr->path << '\n';
            }
           
        }
    }
    
   

}

void quit(AppData *data_ptr)
{
    SDL_DestroyTexture(data_ptr->folder);
    SDL_DestroyTexture(data_ptr->exe);
    SDL_DestroyTexture(data_ptr->image);
    SDL_DestroyTexture(data_ptr->video);
    SDL_DestroyTexture(data_ptr->code);
    SDL_DestroyTexture(data_ptr->other);
    TTF_CloseFont(data_ptr->font);
}

//returns a string with the filepath to the parent directory of the input
//file path
std::string getParent(std::string filepath)
{
    std::string token;
    std::istringstream stream(filepath);
    int j=0;
    std::vector<std::string> sub;
    std::string result = "";

    if(filepath =="")
    {
        return "";
    }

    while (std::getline(stream, token, '/'))
    {
        sub.push_back(token);
        j++;
    }

    if (j==2)
    {
        return "/";
    }
    

    for (int i = 0; i < sub.size()-1; i++)
    {
        
        result = result + sub.at(i);
        if(i < sub.size() -2){
        result = result + "/";
        }
    }
    return result;

}

//used for mouse clicks/exiting the window
bool pollevent(int *x, int *y)
{
    SDL_Event event;

    if (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            _Exit(0);
            break;

        case SDL_MOUSEBUTTONDOWN:
            SDL_GetMouseState(x, y);
            std::cout<< "Mouse was clicked\n";
            return true;
            break;
        
        default:
            break;
        }
    }
    return false;
}

//determines how many sections to divide the window into
//based on the number of items in the directory
int validRegions(int size, AppData* data_ptr){
    int realsize = size -(data_ptr->focus * 8);
    if(size == 8 ||(data_ptr->focus==0 && realsize > 8))
    {
        return 8;
    }
    else
    {
        return realsize;
    }
}

//determines which region of the screen was clicked
//used in figuring out what directory/file to open
int clicked(int x, int y, int regions)
{
    int result;
    if(x < 600)
    {
        result = y/75;
        if(result<=regions-1)
        {
            return result;
        }
    }
    else if (x >= 600 && x < 700 && y>499)
    {
        return 8;
    }
    else if(x>= 700 && x < 800 && y>499)
    {
        return 9;
    }
    return -1;
}

//used in determining appropriate icon
std::string getType(std::string target, AppData *data_ptr)
{
    size_t index = target.rfind(".");
    std::string temp;
    std::string full = "/" + target;
    full = data_ptr->path + full;

    int dir;
    struct stat path_stat;
    stat(full.c_str(), &path_stat);
    dir = S_ISDIR(path_stat.st_mode);



    if(dir!=0){
        return "folder";
    }
    else if(target[0] =='.' && target[1] == '.')
    {
        return "folder";
    }
    else if(index == std::string::npos)
    {
        return "other";
    }
    else
    {
        temp = target.substr(index+1,target.length());
    }

    if(temp == "exe")
    {
        return "exe";
    }

    else if(temp == "jpg" || temp == "jpeg" || temp == "png" || temp == "tif" || temp == "tiff" || temp == "gif")
    {
        return "image";
    }

    else if(temp == "mp4" || temp == "mov" || temp == "mkv" || temp == "avi" || temp == "webm")
    {
        return "video";
    }

    else if(temp == "h" || temp == "c" || temp == "cpp" || temp == "py" || temp == "java" || temp == "js" )
    {
        return "code";
    }
    else
    {
        return "other";
    }

}

