/*
 * Copyright (c) 2015 OpenALPR Technology, Inc.
 * Open source Automated License Plate Recognition [http://www.openalpr.com]
 *
 * This file is part of OpenALPR.
 *
 * OpenALPR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License
 * version 3 as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include "support/filesystem.h"
#include "../tclap/CmdLine.h"
#include "prewarp.h"
#include "alpr.h"
#include "support/filesystem.h"

#include <sstream>

using namespace std;
using namespace cv;
using namespace alpr;

const int INSTRUCTIONS_HEIGHT = 32;


bool panning;
bool left_clicking;
Point left_click_start;
Point left_click_cur;


Point lastPos;

float w;
float h;
float panX = 0;
float panY = 0;
float rotationx = 0;
float rotationy = 0;
float rotationz = 0;
float stretchX = 1.0;
float dist = 1.0;

Mat imgOriginal;
Mat curWarpedImage;

alpr::Config config("eu");

string get_config()
{
  stringstream output;
  output << "planar," << std::fixed;
  output << w << "," << h << ",";
  output << rotationx << "," << rotationy << "," << rotationz << ",";
  output << stretchX << "," << dist << ",";
  output << panX << "," << panY;
  
  return output.str();
}

int main(int argc, char** argv) {


  string country;
  string inDir;
  string outDir;
  string translate_config;
  string prefix;
  
  TCLAP::CmdLine cmd("Batch warping tool", ' ', "0.1");

  TCLAP::ValueArg<std::string> countryCodeArg("c","country","Country code to identify (either us for USA or eu for Europe).  Default=us",false, "us" ,"country_code");
  
  TCLAP::ValueArg<std::string> translateTestArg("t","test","Test an image using the provided translation config", true, "" ,"prewarp config");

  TCLAP::ValueArg<std::string>  inDirArg( "", "in_dir", "Directory with images to warp", true, "./", "in_dir"  );
  TCLAP::ValueArg<std::string>  outDirArg( "", "out_dir", "Directory to output warped images", true, "./", "out_dir"  );
  TCLAP::ValueArg<std::string>  prefixArg( "", "prefix", "prefix to warped images", true, "warped_", "prefix"  );
  
  
  try
  {
    cmd.add( inDirArg );
    cmd.add( outDirArg );
    cmd.add( translateTestArg );
    cmd.add( countryCodeArg );
    cmd.add( prefixArg );
    
    if (cmd.parse( argc, argv ) == false)
    {
      // Error occurred while parsing.  Exit now.
      return 1;
    }

    country = countryCodeArg.getValue();
    inDir = inDirArg.getValue();
    outDir = outDirArg.getValue();
    translate_config = translateTestArg.getValue();
    prefix = prefixArg.getValue();

  }
  catch (TCLAP::ArgException &e)    // catch any exceptions
  {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    return 1;
  }
  
  int first_comma = translate_config.find(",");
    
  string name = translate_config.substr(0, first_comma);
  stringstream ss(translate_config.substr(first_comma + 1, translate_config.length()));
  
  ss >> w;
  ss.ignore();
  ss >> h;
  ss.ignore();
  ss >> rotationx;
  ss.ignore();  // Ignore comma
  ss >> rotationy;
  ss.ignore();  // Ignore comma
  ss >> rotationz;
  ss.ignore();  // Ignore comma
  ss >> stretchX;
  ss.ignore();  // Ignore comma
  ss >> dist;
  ss.ignore();  // Ignore comma
  ss >> panX;
  ss.ignore();  // Ignore comma
  ss >> panY;
       
  config = alpr::Config(country);
  config.prewarp = get_config();
  
  alpr::PreWarp prewarp(&config);
  cerr << "Applying warping to files in " << inDir << endl;
  vector<string> files = getFilesInDir(inDir.c_str(), true);
  for (vector<string>::const_iterator iter = files.begin(); iter != files.end(); ++iter)
  {
      const std::string& f = *iter;
      if (hasEnding(f, ".png") || hasEnding(f, ".jpg") || hasEnding(f, ".jpeg"))
      {
        string fullpath = inDir + "/" + f;
        string outfullpath = outDir + "/" + prefix + f;
        cout << "Warping image " << fullpath << " in " <<  outfullpath << endl;
        Mat img = imread(fullpath.c_str());
        Mat output_image = prewarp.warpImage(img);
  
        cv::imwrite(outfullpath.c_str(), output_image);
      }
  }
  
  return 0;
}
