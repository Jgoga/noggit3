// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#include <noggit/scripting/script_image.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_exception.hpp>

#include <sol/sol.hpp>
#include <lodepng.h>

namespace noggit
{
  namespace scripting
  {
    void image::resize(int width, int height)
    {
        if(width<=0||height<=0)
        {
          throw script_exception(
            "img_resize",
            std::string("tried to resize to invalid image size: x=")
            + std::to_string(width)
            + std::string(" y=")
            + std::to_string(height)
          );
        }
        _size = width*height*4;
        _width = width;
        _height = height;
    }

    image::image(char const* path)
    {
      if(path==nullptr)
      {
          throw script_exception("load_png","empty png path");
      }
      unsigned error = lodepng::decode(_image, _width, _height, path);
      if (error)
      {
        throw script_exception(
          "img_load_png",
          "failed to load png image with error code:"
          + std::to_string (error));
      }
      resize(_width, _height);
    }

    image::image(int width, int height)
    {
      resize(width,height);
    }

    int image::width()
    {
      return _width;
    }

    int image::height()
    {
      return _height;
    }

    int image::get_index(int x, int y)
    {
      int index = ((x + y * _width) * 4);
      if(index<0||index>=_size)
      {
        throw script_exception(
          "img_get_index",
          "image coordinates out of bounds: x="
            + std::to_string(x)
            + " y="
            + std::to_string(y)
            + " width="
            + std::to_string(_width)
            + " height="
            + std::to_string(_height));
      }
      return index;
    }

    unsigned image::get_pixel(int x, int y)
    {
      unsigned index = get_index(x, y);
      return get_image()[index] << 24
        | get_image()[index + 1] << 16
        | get_image()[index + 2] << 8
        | get_image()[index + 3];
    }

    void image::set_pixel(int x, int y, unsigned value)
    {
      unsigned index = get_index(x, y);
      get_image()[index] = (value << 24);
      get_image()[index + 1] = (value << 16) & 0xff;
      get_image()[index + 2] = (value << 8) & 0xff;
      get_image()[index + 3] = (value) & 0xff;
    }

    void image::save(char const* filename)
    {
      unsigned error = lodepng::encode(filename, get_image(), _width, _height);
      if (error)
      {
        throw script_exception(
          "img_save",
          "failed to save image with error "
          + std::to_string (error));
      }
    }

    float image::gradient_scale(float rel)
    {
      if(rel<0||rel>=1)
      {
        throw script_exception(
          "img_gradient_scale",
          "relative image coordinate out of bounds: "
            + std::to_string(rel)
            + " (should be >= 0 and < 1)");
      }
      int x = std::floor(rel * float(_width));
      // read red channel, but it shouldn't matter.
      return float(get_image()[x * 4]) / 255.0;
    }

    image create_image(int width ,int height)
    {
      return image(width, height);
    }

    image load_png(const char* path)
    {
      return image(path);
    }

    void register_image(sol::state * state, scripting_tool * tool)
    {
      state->new_usertype<image>("image"
        , "get_index", &image::get_index
        , "get_pixel", &image::get_pixel
        , "gradient_scale", &image::gradient_scale
        , "set_pixel", &image::set_pixel
        , "save", &image::save
        , "width", &image::width
        , "height", &image::height
      );

      state->set_function("create_image",create_image);
      state->set_function("load_png",load_png);
    }
  } // namespace scripting
} // namespace noggit
