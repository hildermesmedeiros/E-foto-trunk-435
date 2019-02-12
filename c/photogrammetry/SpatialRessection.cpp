/**************************************************************************
      SpatialRessection.cpp
**************************************************************************/
/*Copyright 2002-2014 e-foto team (UERJ)
  This file is part of e-foto.

    e-foto is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    e-foto is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with e-foto.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef WIN32
#define _USE_MATH_DEFINES // for C++
#include <cmath>
#endif
#ifdef unix
#include <math.h>
#endif

#include "SpatialRessection.h"
#include "Image.h"
#include "Point.h"
#include "InteriorOrientation.h"
#include "Sensor.h"
#include "FrameSensor.h"
#include "Terrain.h"
#include "Flight.h"
#include "RayTester.h"

#include "math.h"

#include <QDebug>
#include <sstream>

namespace br {
namespace uerj {
namespace eng {
namespace efoto {

// Constructors and destructors
//

/**
 *
 */
SpatialRessection::SpatialRessection()
{
    pointForFlightDirectionAvailable = false;
    flightDirectionAvailable = false;
    totalIterations = 0;
    gnssConverged = false;
    insConverged = false;
    useDistortions = true;

    if (myImage != nullptr){
        rt = new RayTester(myImage);
    }
    else {
        rt = nullptr;
    }

}

/**
 *
 */
SpatialRessection::SpatialRessection(int myImageId) // Constructor with ids only, needed in project use.
{
    imageId = myImageId;
    pointForFlightDirectionAvailable = flightDirectionAvailable = false;
    totalIterations = 0;
    gnssConverged = false;
    insConverged = false;
    //  myImage = image(myImageId);
    if (myImage != nullptr){
        rt = new RayTester(myImage);
    }
    else {
        rt = nullptr;
    }
    useDistortions = true;
}

/**
 *
 */
SpatialRessection::~SpatialRessection()
{
    if (rt != nullptr)
    {
        delete rt;
        rt = nullptr;
    }
}


// Private attribute accessors
//

/**
 * Set the value of Xa
 * This method is only used by the Mounter.
 * @param newXa the new value of Xa
 */
void SpatialRessection::setXa(const Matrix& newXa)
{
    Xa = newXa;
}

/**
 * Set the value of La
 * This method is only used by the Mounter.
 * @param newLa the new value of La
 */
void SpatialRessection::setLa(const Matrix& newLa)
{
    La = newLa;
}

/**
 * Set the value of A
 * This method is only used by the Mounter.
 * @param newA the new value of A
 */
void SpatialRessection::setA(const Matrix& newA)
{
    A = newA;
}

/**
 * Set the value of P
 * This method is only used by the Mounter.
 * @param newP the new value of P
 */
void SpatialRessection::setP(const Matrix& newP)
{
    P = newP;
}

/**
 * Set the value of X0
 * This method is only used by the Mounter.
 * @param newX0 the new value of X0
 */
void SpatialRessection::setX0(const Matrix& newX0)
{
    X0 = newX0;
}

/**
 * Set the value of L0
 * This method is only used by the Mounter.
 * @param newL0 the new value of L0
 */
void SpatialRessection::setL0(const Matrix& newL0)
{
    L0 = newL0;
}

/**
 * Set the value of Lb
 * This method is only used by the Mounter.
 * @param newLb the new value of Lb
 */
void SpatialRessection::setLb(const Matrix& newLb)
{
    Lb = newLb;
}

/**
 * Get the value of La
 * @return the value of La
 */
Matrix SpatialRessection::getLa()
{
    return La;
}

/**
 * Get the value of A
 * @return the value of A
 */
Matrix SpatialRessection::getA()
{
    return A;
}

/**
 * Get the value of P
 * @return the value of P
 */
Matrix SpatialRessection::getP()
{
    return P;
}

/**
 * Get the value of X0
 * @return the value of X0
 */
Matrix SpatialRessection::getX0()
{
    return X0;
}

/**
 * Get the value of L0
 * @return the value of L0
 */
Matrix SpatialRessection::getL0()
{
    return L0;
}

/**
 * Get the value of Lb
 * @return the value of Lb
 */
Matrix SpatialRessection::getLb()
{
    return Lb;
}

/**
 * Get the value of lastL0
 * @return the value of lastL0
 */
Matrix SpatialRessection::getLastL0()
{
    return lastL0;
}

/**
 *
 */
std::deque<int> SpatialRessection::getSelectedPoints()
{
    return selectedPoints;
}

/**
 *
 */
ImageSpaceCoordinate* SpatialRessection::getPointForFlightDirection()
{
    if (pointForFlightDirectionAvailable)
        return &pointForFlightDirection;
    else
        return NULL;
}

bool SpatialRessection::getConverged()
{
    return gnssConverged && insConverged;
}

bool SpatialRessection::getGnssConverged()
{
    return gnssConverged;
}

bool SpatialRessection::getInsConverged()
{
    return insConverged;
}

// Selected points list manipulators
//

void SpatialRessection::selectPoint(int id)
{
    bool insert = true;
    for (unsigned int i = 0; i < selectedPoints.size(); i++)
        if (selectedPoints.at(i) == id)
            insert = false;
    if (insert)
        selectedPoints.push_back(id);
}

void SpatialRessection::unselectPoint(int id)
{
    for (unsigned int i = 0; i < selectedPoints.size(); i++)
        if (selectedPoints.at(i) == id)
            selectedPoints.erase(selectedPoints.begin() + i);
}

void SpatialRessection::unselectAllPoints()
{
    selectedPoints.clear();
}

int SpatialRessection::countSelectedPoints()
{
    return selectedPoints.size();
}

// Selected fiducial mark or point to indicate the direction of flight manipulators
//

void SpatialRessection::setFlightDirection(double kappa0)
{
    myImage->setFlightDirection(kappa0);
    flightDirectionAvailable = true;
    pointForFlightDirectionAvailable = false;
}

void SpatialRessection::setPointForFlightDirection(double col, double lin)
{
    pointForFlightDirection.setCol(col);
    pointForFlightDirection.setLin(lin);
    flightDirectionAvailable = false;
    pointForFlightDirectionAvailable = true;
}

void SpatialRessection::selectFiducialMarkForFlightDirection(int id)
{
    if (myImage != NULL)
    {
        pointForFlightDirection.setCol(myImage->getDigFidMark(id).getCol());
        pointForFlightDirection.setLin(myImage->getDigFidMark(id).getLin());
        flightDirectionAvailable = false;
        pointForFlightDirectionAvailable = true;
    }
}

void SpatialRessection::unsetPointForFlightDirection()
{
    flightDirectionAvailable = false;
}

// EObject methods
//

/**
 *
 */
std::string SpatialRessection::objectType(void)
{
    std::stringstream result;
    result << "SpatialResection " << imageId;
    return result.str();
}

/**
 *
 */
std::string SpatialRessection::objectAssociations(void)
{
    return myImage->objectType();
}

/**
 *
 */
bool SpatialRessection::is(std::string s)
{
    return (s == "SpatialResection" ? true : false);
}

// XML methods
//

/**
 *
 */
void SpatialRessection::xmlSetData(std::string xml)
{
    EDomElement root(xml);
    imageId = Conversion::stringToInt(root.attribute("image_key"));
    totalIterations = root.elementByTagName("iterations").toInt();
    if (root.elementByTagName("converged").toString() == "true")
        gnssConverged = insConverged = true;
    else
        gnssConverged = insConverged = false;

    std::deque<EDomElement> pts = root.elementByTagName("usedPoints").children();
    selectedPoints.clear();
    for(int i = 0; i < (int)pts.size();i++)
    {
        selectedPoints.push_back(pts.at(i).elementByTagName("pointKey").toInt());
    }

    Lb.xmlSetData(root.elementByTagName("Lb").elementByTagName("mml:matrix").getContent());

    Xa.resize(6,1);
    EDomElement xmlXa = root.elementByTagName("Xa");
    Xa.set(1,1, xmlXa.elementByTagName("X0").toDouble());
    Xa.set(2,1, xmlXa.elementByTagName("Y0").toDouble());
    Xa.set(3,1, xmlXa.elementByTagName("Z0").toDouble());
    Xa.set(4,1, xmlXa.elementByTagName("phi").toDouble());
    Xa.set(5,1, xmlXa.elementByTagName("omega").toDouble());
    Xa.set(6,1, xmlXa.elementByTagName("kappa").toDouble());

    L0.xmlSetData(root.elementByTagName("L0").elementByTagName("mml:matrix").getContent());

    X0.resize(6,1);
    // Aqui vem a maldição do nome igual. Tem um elemento X0 dentro do Xa, e um X0 fora. Quero achar o fora.
    // Eu poderia fazer o acesso diretamente pelo nome dos elementos internos, mas seria dar muitas voltas no XML.
    EDomElement xmlX0;
    std::deque<EDomElement> xmlX0s = root.elementsByTagName("X0");
    for (unsigned int i = 0; i < xmlX0s.size(); i++)
        if (xmlX0s.at(i).children().size() == 6)
            xmlX0 = xmlX0s.at(i);
    X0.set(1,1, xmlX0.elementByTagName("X00").toDouble());
    X0.set(2,1, xmlX0.elementByTagName("Y00").toDouble());
    X0.set(3,1, xmlX0.elementByTagName("Z00").toDouble());
    X0.set(4,1, xmlX0.elementByTagName("phi0").toDouble());
    X0.set(5,1, xmlX0.elementByTagName("omega0").toDouble());
    X0.set(6,1, xmlX0.elementByTagName("kappa0").toDouble());

    myQuality.xmlSetData(root.elementByTagName("quality").getContent());
}

/**
 *
 */
std::string SpatialRessection::xmlGetData()
{
    std::stringstream result;
    result << "<imageSR image_key=\"" << Conversion::intToString(imageId) << "\">\n";
    result << "<iterations>" << Conversion::intToString(totalIterations) << "</iterations>\n";
    if (gnssConverged && insConverged)
        result << "<converged>true</converged>\n";
    else
        result << "<converged>false</converged>\n";
    result << "<parameters>\n";

    result << "<usedPoints>\n";
    for (int i=0;i<(int)selectedPoints.size();i++)
    {
        result << "<pointKey>" <<	Conversion::intToString(selectedPoints.at(i))	<<"</pointKey>\n";
    }
    result << "</usedPoints>\n";

    result << "<Lb>\n";
    result << Lb.xmlGetData();
    result << "</Lb>\n";
    result << "<L0>\n";
    result << L0.xmlGetData();
    result << "</L0>\n";
    result << "<X0>\n";
    result << "<X00 uom=\"#m\">" << Conversion::doubleToString(X0.get(1,1)) << "</X00>\n";
    result << "<Y00 uom=\"#m\">" << Conversion::doubleToString(X0.get(2,1)) << "</Y00>\n";
    result << "<Z00 uom=\"#m\">" << Conversion::doubleToString(X0.get(3,1)) << "</Z00>\n";
    result << "<phi0 uom=\"#rad\">" << Conversion::doubleToString(X0.get(4,1)) << "</phi0>\n";
    result << "<omega0 uom=\"#rad\">" << Conversion::doubleToString(X0.get(5,1)) << "</omega0>\n";
    result << "<kappa0 uom=\"#rad\">" << Conversion::doubleToString(X0.get(6,1)) << "</kappa0>\n";
    result << "</X0>\n";
    result << "</parameters>\n";
    result << myQuality.xmlGetData();
    result << "</imageSR>\n";
    return result.str();
}

std::string SpatialRessection::xmlGetDataEO()
{
    std::stringstream result;
    result << "<imageEO type=\"spatialResection\" image_key=\"" << Conversion::intToString(imageId) << "\">\n";
    result << "<Xa>\n";
    result << "<X0 uom=\"#m\">" << Conversion::doubleToString(Xa.get(1,1)) << "</X0>\n";
    result << "<Y0 uom=\"#m\">" << Conversion::doubleToString(Xa.get(2,1)) << "</Y0>\n";
    result << "<Z0 uom=\"#m\">" << Conversion::doubleToString(Xa.get(3,1)) << "</Z0>\n";
    result << "<phi uom=\"#rad\">" << Conversion::doubleToString(Xa.get(4,1)) << "</phi>\n";
    result << "<omega uom=\"#rad\">" << Conversion::doubleToString(Xa.get(5,1)) << "</omega>\n";
    result << "<kappa uom=\"#rad\">" << Conversion::doubleToString(Xa.get(6,1)) << "</kappa>\n";
    result << "</Xa>\n";
    result << "</imageEO>\n";
    return result.str();
}

// Other methods
//

/*//////////////////////////////////////////////////////////////////////////////////////////////////
  Aqui tem codigo da PR
  ////////////////////////////////////////////////////////////////////////////////////////////////*/

/*void SpatialRessection::generateR()
{
 r11 = cos(phi0) * cos(kappa0);
 r12 = -cos(phi0) * sin(kappa0);
 r13 = sin(phi0);
 r21 = cos(omega0) * sin(kappa0) + sin(omega0) * sin(phi0) * cos(kappa0);
 r22 = cos(omega0) * cos(kappa0) - sin(omega0) * sin(phi0) * sin(kappa0);
 r23 = -sin(omega0) * cos(phi0);
 r31 = sin(omega0) * sin(kappa0) - cos(omega0) * sin(phi0) * cos(kappa0);
 r32 = sin(omega0) * cos(kappa0) + cos(omega0) * sin(phi0) * sin(kappa0);
 r33 = cos(omega0) * cos(phi0);
}

110627 - The R matrix is now calculated inside the RayTester class.

*/

/*//////////////////////////////////////////////////////////////////////////////////////////////////
  Aqui tem codigo da PR
  ////////////////////////////////////////////////////////////////////////////////////////////////*/

void SpatialRessection::generateInitialA()
{
    if (myImage != NULL)
    {
        A.resize(selectedPoints.size() * 2, 6).zero();
        for (unsigned int i = 0, j = 1; i < selectedPoints.size(); i++, j+=2)
        {
            // Essas linhas estão horríveis de ler!!! Mas não consigo melhorar mais que isso...
            Point* myPoint = myImage->getPoint(selectedPoints.at(i)); // Escolhi aqui o ponto que estou usando.
            DetectorSpaceCoordinate myCoordinate = myPoint->getDetectorCoordinate(myImage->getId()); // Tento tirar as coordenadas analógicas dele.
            if (myCoordinate.getUnit() == "") // Se essas coordenadas analógicas não existirem, acusadas pela falta de unidade...
                myCoordinate = rt->imageToDetector(myPoint->getImageCoordinate(myImage->getId()));
            //myCoordinate = myImage->getIO()->digitalToAnalog(myPoint->getDigitalCoordinate(myImage->getId())); // Crio elas usando digitalToAnalog nas coordenadas digitais.

            // Distortions added.
            //if (useDistortions)
            //myCoordinate = applyDistortions(myCoordinate);
            //myCoordinate = removeDistortions(myCoordinate);

            A.set(j,1,1.0);
            A.set(j,2,myCoordinate.getXi());
            A.set(j,3,myCoordinate.getEta());
            A.set(j+1,4,1.0);
            A.set(j+1,5,myCoordinate.getXi());
            A.set(j+1,6,myCoordinate.getEta());
        }
    }
}

void SpatialRessection::generateInitialL0()
{
    if (myImage != NULL)
    {
        L0.resize(selectedPoints.size() * 2, 1);
        for (unsigned int i = 0, j = 1; i < selectedPoints.size(); i++, j+=2)
        {
            Point* myPoint = myImage->getPoint(selectedPoints.at(i));
            ObjectSpaceCoordinate myCoordinate = myPoint->getObjectCoordinate();

            L0.set(j,1,myCoordinate.getX());
            L0.set(j+1,1,myCoordinate.getY());
        }
    }
}

void SpatialRessection::generateInitialP()
{
    P.identity(selectedPoints.size() * 2);
}

void SpatialRessection::generateA()
{
    if (myImage != NULL)
    {
        A.resize(selectedPoints.size() * 2, 6);
        double c = myImage->getSensor()->getFocalDistance();

        for (unsigned int i = 0, j = 1; i < selectedPoints.size(); i++, j+=2)
        {
            Point* myPoint = myImage->getPoint(selectedPoints.at(i));
            ObjectSpaceCoordinate myCoordinate = myPoint->getObjectCoordinate();
            double X = myCoordinate.getX();
            double Y = myCoordinate.getY();
            double Z = myCoordinate.getZ();

            A.set(j,1,c*cos(kappa0)*cos(phi0)/(sin(phi0)*(X-X00)-cos(phi0)*sin(omega0)*(Y-Y00)+cos(phi0)*cos(omega0)*(Z-Z00))-c*(cos(kappa0)*cos(phi0)*(X-X00)+(sin(kappa0)*cos(omega0)+cos(kappa0)*sin(phi0)*sin(omega0))*(Y-Y00)+(sin(kappa0)*sin(omega0)-cos(kappa0)*sin(phi0)*cos(omega0))*(Z-Z00))/pow((sin(phi0)*(X-X00)-cos(phi0)*sin(omega0)*(Y-Y00)+cos(phi0)*cos(omega0)*(Z-Z00)),2)*sin(phi0));
            A.set(j,2,-c*(-sin(kappa0)*cos(omega0)-cos(kappa0)*sin(phi0)*sin(omega0))/(sin(phi0)*(X-X00)-cos(phi0)*sin(omega0)*(Y-Y00)+cos(phi0)*cos(omega0)*(Z-Z00))+c*(cos(kappa0)*cos(phi0)*(X-X00)+(sin(kappa0)*cos(omega0)+cos(kappa0)*sin(phi0)*sin(omega0))*(Y-Y00)+(sin(kappa0)*sin(omega0)-cos(kappa0)*sin(phi0)*cos(omega0))*(Z-Z00))/pow((sin(phi0)*(X-X00)-cos(phi0)*sin(omega0)*(Y-Y00)+cos(phi0)*cos(omega0)*(Z-Z00)),2)*cos(phi0)*sin(omega0));
            A.set(j,3,-c*(-sin(kappa0)*sin(omega0)+cos(kappa0)*sin(phi0)*cos(omega0))/(sin(phi0)*(X-X00)-cos(phi0)*sin(omega0)*(Y-Y00)+cos(phi0)*cos(omega0)*(Z-Z00))-c*(cos(kappa0)*cos(phi0)*(X-X00)+(sin(kappa0)*cos(omega0)+cos(kappa0)*sin(phi0)*sin(omega0))*(Y-Y00)+(sin(kappa0)*sin(omega0)-cos(kappa0)*sin(phi0)*cos(omega0))*(Z-Z00))/pow((sin(phi0)*(X-X00)-cos(phi0)*sin(omega0)*(Y-Y00)+cos(phi0)*cos(omega0)*(Z-Z00)),2)*cos(phi0)*cos(omega0));
            A.set(j,4,-c*(-cos(kappa0)*sin(phi0)*(X-X00)+cos(kappa0)*cos(phi0)*sin(omega0)*(Y-Y00)-cos(kappa0)*cos(phi0)*cos(omega0)*(Z-Z00))/(sin(phi0)*(X-X00)-cos(phi0)*sin(omega0)*(Y-Y00)+cos(phi0)*cos(omega0)*(Z-Z00))+c*(cos(kappa0)*cos(phi0)*(X-X00)+(sin(kappa0)*cos(omega0)+cos(kappa0)*sin(phi0)*sin(omega0))*(Y-Y00)+(sin(kappa0)*sin(omega0)-cos(kappa0)*sin(phi0)*cos(omega0))*(Z-Z00))/pow((sin(phi0)*(X-X00)-cos(phi0)*sin(omega0)*(Y-Y00)+cos(phi0)*cos(omega0)*(Z-Z00)),2)*(cos(phi0)*(X-X00)+sin(phi0)*sin(omega0)*(Y-Y00)-sin(phi0)*cos(omega0)*(Z-Z00)));
            A.set(j,5,-c*((-sin(kappa0)*sin(omega0)+cos(kappa0)*sin(phi0)*cos(omega0))*(Y-Y00)+(sin(kappa0)*cos(omega0)+cos(kappa0)*sin(phi0)*sin(omega0))*(Z-Z00))/(sin(phi0)*(X-X00)-cos(phi0)*sin(omega0)*(Y-Y00)+cos(phi0)*cos(omega0)*(Z-Z00))+c*(cos(kappa0)*cos(phi0)*(X-X00)+(sin(kappa0)*cos(omega0)+cos(kappa0)*sin(phi0)*sin(omega0))*(Y-Y00)+(sin(kappa0)*sin(omega0)-cos(kappa0)*sin(phi0)*cos(omega0))*(Z-Z00))/pow((sin(phi0)*(X-X00)-cos(phi0)*sin(omega0)*(Y-Y00)+cos(phi0)*cos(omega0)*(Z-Z00)),2)*(-cos(phi0)*cos(omega0)*(Y-Y00)-cos(phi0)*sin(omega0)*(Z-Z00)));
            A.set(j,6,-c*(-sin(kappa0)*cos(phi0)*(X-X00)+(cos(kappa0)*cos(omega0)-sin(kappa0)*sin(phi0)*sin(omega0))*(Y-Y00)+(cos(kappa0)*sin(omega0)+sin(kappa0)*sin(phi0)*cos(omega0))*(Z-Z00))/(sin(phi0)*(X-X00)-cos(phi0)*sin(omega0)*(Y-Y00)+cos(phi0)*cos(omega0)*(Z-Z00)));

            A.set(j+1,1,-c*sin(kappa0)*cos(phi0)/(sin(phi0)*(X-X00)-cos(phi0)*sin(omega0)*(Y-Y00)+cos(phi0)*cos(omega0)*(Z-Z00))-c*(-sin(kappa0)*cos(phi0)*(X-X00)+(cos(kappa0)*cos(omega0)-sin(kappa0)*sin(phi0)*sin(omega0))*(Y-Y00)+(cos(kappa0)*sin(omega0)+sin(kappa0)*sin(phi0)*cos(omega0))*(Z-Z00))/pow((sin(phi0)*(X-X00)-cos(phi0)*sin(omega0)*(Y-Y00)+cos(phi0)*cos(omega0)*(Z-Z00)),2)*sin(phi0));
            A.set(j+1,2,-c*(-cos(kappa0)*cos(omega0)+sin(kappa0)*sin(phi0)*sin(omega0))/(sin(phi0)*(X-X00)-cos(phi0)*sin(omega0)*(Y-Y00)+cos(phi0)*cos(omega0)*(Z-Z00))+c*(-sin(kappa0)*cos(phi0)*(X-X00)+(cos(kappa0)*cos(omega0)-sin(kappa0)*sin(phi0)*sin(omega0))*(Y-Y00)+(cos(kappa0)*sin(omega0)+sin(kappa0)*sin(phi0)*cos(omega0))*(Z-Z00))/pow((sin(phi0)*(X-X00)-cos(phi0)*sin(omega0)*(Y-Y00)+cos(phi0)*cos(omega0)*(Z-Z00)),2)*cos(phi0)*sin(omega0));
            A.set(j+1,3,-c*(-cos(kappa0)*sin(omega0)-sin(kappa0)*sin(phi0)*cos(omega0))/(sin(phi0)*(X-X00)-cos(phi0)*sin(omega0)*(Y-Y00)+cos(phi0)*cos(omega0)*(Z-Z00))-c*(-sin(kappa0)*cos(phi0)*(X-X00)+(cos(kappa0)*cos(omega0)-sin(kappa0)*sin(phi0)*sin(omega0))*(Y-Y00)+(cos(kappa0)*sin(omega0)+sin(kappa0)*sin(phi0)*cos(omega0))*(Z-Z00))/pow((sin(phi0)*(X-X00)-cos(phi0)*sin(omega0)*(Y-Y00)+cos(phi0)*cos(omega0)*(Z-Z00)),2)*cos(phi0)*cos(omega0));
            A.set(j+1,4,-c*(sin(kappa0)*sin(phi0)*(X-X00)-sin(kappa0)*cos(phi0)*sin(omega0)*(Y-Y00)+sin(kappa0)*cos(phi0)*cos(omega0)*(Z-Z00))/(sin(phi0)*(X-X00)-cos(phi0)*sin(omega0)*(Y-Y00)+cos(phi0)*cos(omega0)*(Z-Z00))+c*(-sin(kappa0)*cos(phi0)*(X-X00)+(cos(kappa0)*cos(omega0)-sin(kappa0)*sin(phi0)*sin(omega0))*(Y-Y00)+(cos(kappa0)*sin(omega0)+sin(kappa0)*sin(phi0)*cos(omega0))*(Z-Z00))/pow((sin(phi0)*(X-X00)-cos(phi0)*sin(omega0)*(Y-Y00)+cos(phi0)*cos(omega0)*(Z-Z00)),2)*(cos(phi0)*(X-X00)+sin(phi0)*sin(omega0)*(Y-Y00)-sin(phi0)*cos(omega0)*(Z-Z00)));
            A.set(j+1,5,-c*((-cos(kappa0)*sin(omega0)-sin(kappa0)*sin(phi0)*cos(omega0))*(Y-Y00)+(cos(kappa0)*cos(omega0)-sin(kappa0)*sin(phi0)*sin(omega0))*(Z-Z00))/(sin(phi0)*(X-X00)-cos(phi0)*sin(omega0)*(Y-Y00)+cos(phi0)*cos(omega0)*(Z-Z00))+c*(-sin(kappa0)*cos(phi0)*(X-X00)+(cos(kappa0)*cos(omega0)-sin(kappa0)*sin(phi0)*sin(omega0))*(Y-Y00)+(cos(kappa0)*sin(omega0)+sin(kappa0)*sin(phi0)*cos(omega0))*(Z-Z00))/pow((sin(phi0)*(X-X00)-cos(phi0)*sin(omega0)*(Y-Y00)+cos(phi0)*cos(omega0)*(Z-Z00)),2)*(-cos(phi0)*cos(omega0)*(Y-Y00)-cos(phi0)*sin(omega0)*(Z-Z00)));
            A.set(j+1,6,-c*(-cos(kappa0)*cos(phi0)*(X-X00)+(-sin(kappa0)*cos(omega0)-cos(kappa0)*sin(phi0)*sin(omega0))*(Y-Y00)+(-sin(kappa0)*sin(omega0)+cos(kappa0)*sin(phi0)*cos(omega0))*(Z-Z00))/(sin(phi0)*(X-X00)-cos(phi0)*sin(omega0)*(Y-Y00)+cos(phi0)*cos(omega0)*(Z-Z00)));
        }
    }
}

/*//////////////////////////////////////////////////////////////////////////////////////////////////
  Aqui tem codigo da PR
  ////////////////////////////////////////////////////////////////////////////////////////////////*/

void SpatialRessection::generateL0()
{
    if (myImage != NULL)
    {
        L0.resize(selectedPoints.size() * 2, 1);
        //double c = myImage->getSensor()->getFocalDistance();
        //double xi0 = myImage->getSensor()->getPrincipalPointCoordinates().getXi();
        //double eta0 = myImage->getSensor()->getPrincipalPointCoordinates().getEta();

        //generateR();

        rt->setEOParameters(X0);

        for (unsigned int i = 0, j = 1; i < selectedPoints.size(); i++, j+=2)
        {
            Point* myPoint = myImage->getPoint(selectedPoints.at(i));
            ObjectSpaceCoordinate myCoordinate = myPoint->getObjectCoordinate();

            //double X = myCoordinate.getX();
            //double Y = myCoordinate.getY();
            //double Z = myCoordinate.getZ();

            //double L0xi = xi0-c*(r11*(X-X00)+r21*(Y-Y00)+r31*(Z-Z00))/(r13*(X-X00)+r23*(Y-Y00)+r33*(Z-Z00));
            //double L0eta = eta0-c*(r12*(X-X00)+r22*(Y-Y00)+r32*(Z-Z00))/(r13*(X-X00)+r23*(Y-Y00)+r33*(Z-Z00));

            //DetectorSpaceCoordinate newXiEta = applyDistortions(L0xi, L0eta);
            //DetectorSpaceCoordinate newXiEta = removeDistortions(L0xi, L0eta);

            //double newXi = newXiEta.getXi();
            //double newEta = newXiEta.getEta();

            //L0.set(j,1,newXi);
            //L0.set(j+1,1,newEta);

            //L0.set(j,1,xi0-c*(r11*(X-X00)+r21*(Y-Y00)+r31*(Z-Z00))/(r13*(X-X00)+r23*(Y-Y00)+r33*(Z-Z00)));
            //L0.set(j+1,1,eta0-c*(r12*(X-X00)+r22*(Y-Y00)+r32*(Z-Z00))/(r13*(X-X00)+r23*(Y-Y00)+r33*(Z-Z00)));

            DetectorSpaceCoordinate analog = rt->objectToDetector(myCoordinate);

            L0.set(j, 1, analog.getXi());
            L0.set(j+1, 1, analog.getEta());
        }
    }
}

void SpatialRessection::generateRMSE()
{
    double sum = 0;
    for (int i = 0; i < (int)L0.getRows(); i++)
    {
        sum += pow(L0.get(i+1,1),2);
        //qDebug("%f",sum);
    }
    //qDebug("%f",sqrt(sum/(selectedPoints.size()*2-6)));
    rmse.push_back(sqrt(sum/(selectedPoints.size()*2-6)));
}

/*//////////////////////////////////////////////////////////////////////////////////////////////////
  Aqui tem codigo da PR
  ////////////////////////////////////////////////////////////////////////////////////////////////*/

void SpatialRessection::generateLb()
{
    if (myImage != NULL)
    {
        Lb.resize(selectedPoints.size() * 2, 1);
        for (unsigned int i = 0, j = 1; i < selectedPoints.size(); i++, j+=2)
        {
            // Mesmo código horrível do A inicial.
            Point* myPoint = myImage->getPoint(selectedPoints.at(i));
            DetectorSpaceCoordinate myCoordinate = myPoint->getDetectorCoordinate(myImage->getId());
            if (myCoordinate.getUnit() == "")
                myCoordinate = rt->imageToDetector(myPoint->getImageCoordinate(myImage->getId()));
            //myCoordinate = myImage->getIO()->digitalToAnalog(myPoint->getDigitalCoordinate(myImage->getId()));

            // Distortions added.
            //if (useDistortions)
            //myCoordinate = applyDistortions(myCoordinate);
            //myCoordinate = removeDistortions(myCoordinate);

            Lb.set(j,1,myCoordinate.getXi());
            Lb.set(j+1,1,myCoordinate.getEta());
        }
    }
}

void SpatialRessection::generateP()
{
    P.identity(selectedPoints.size() * 2);
}

void SpatialRessection::generateX0()
{
    X0 = Xa;
    X00 = X0.get(1,1);
    Y00 = X0.get(2,1);
    Z00 = X0.get(3,1);
    phi0 = X0.get(4,1);
    omega0 = X0.get(5,1);
    kappa0 = X0.get(6,1);
}

/*//////////////////////////////////////////////////////////////////////////////////////////////////
  Aqui tem codigo da PR
  ////////////////////////////////////////////////////////////////////////////////////////////////*/

void SpatialRessection::initialize()
{
    if (myImage != NULL && myImage->getSensor() != NULL && myImage->getFlight() != NULL && myImage->getIO() != NULL && (pointForFlightDirectionAvailable || flightDirectionAvailable))
    {
        if (rt == NULL)
            rt = new RayTester(myImage);
        else
            rt->setImage(myImage);

        rt->setIOParameters(myImage->getIO()->getXa());

        generateInitialA();
        generateInitialL0();
        generateInitialP();

        // Calculating X00 and Y00.

        double xi0 = myImage->getSensor()->getPrincipalPointCoordinates().getXi();
        double eta0 = myImage->getSensor()->getPrincipalPointCoordinates().getEta();

        // Warning: this Xa is NOT the one containing the EO's parameters.
        // It's just used for determining the initial X00 and Y00 and will be erased at the end of initialization.
        Matrix Xa = (A.transpose() * P * A).inverse() * A.transpose() * P * L0;
        X00 = Xa.get(1,1) + xi0 * Xa.get(2,1) + eta0 * Xa.get(3,1);
        Y00 = Xa.get(4,1) + xi0 * Xa.get(5,1) + eta0 * Xa.get(6,1);

        // Z00 comes from flight height.

        // If flightHeight is the flight's height, this is correct.
        /*
  double mediumPointHeight = 0;
  for (unsigned int i = 0; i < selectedPoints.size(); i++)
   mediumPointHeight += myImage->getPoint(selectedPoints.at(i))->getObjectCoordinate().getZ();
  mediumPointHeight /= (selectedPoints.size());
  Z00 = myImage->getFlight()->getHeight() + mediumPointHeight;
  */

        //or
        double meanAltitude = myImage->getFlight()->getTerrain()->getMeanAltitude();
        Z00 = myImage->getFlight()->getScaleDen()*myImage->getSensor()->getFocalDistance()/1000 + meanAltitude;

        // If flightHeight is the flight's altitude, this is correct.
        //Z00 = myImage->getFlight()->getHeight();

        // Omega0 and phi0 are initially set to 0.
        omega0 = 0;
        phi0 = 0;

        if (flightDirectionAvailable)
            kappa0 = myImage->getFlightDirection();
        else
        {
            // Calculating kappa0.
            DetectorSpaceCoordinate fiducialCoordinate = rt->imageToDetector(pointForFlightDirection.getCol(),pointForFlightDirection.getLin());

            double fiducialXi = fiducialCoordinate.getXi();
            double fiducialEta = fiducialCoordinate.getEta();

            double fiducialX = Xa.get(1,1) + fiducialXi * Xa.get(2,1) + fiducialEta * Xa.get(3,1);
            double fiducialY = Xa.get(4,1) + fiducialXi * Xa.get(5,1) + fiducialEta * Xa.get(6,1);

            double deltaX = fiducialX - X00;
            double deltaY = fiducialY - Y00;

            double angle = atan(fabs(deltaY/deltaX));
            if ((deltaX >= 0.0) && (deltaY >= 0.0))
                kappa0 = angle;
            if ((deltaX < 0.0) && (deltaY >= 0.0))
                kappa0 = M_PI - angle;
            if ((deltaX < 0.0) && (deltaY < 0.0))
                kappa0 = M_PI + angle;
            if ((deltaX >= 0.0) && (deltaY < 0.0))
                kappa0 = -angle;
        }

        // Setting the values to X0 and reseting Xa.
        X0.resize(6, 1);

        if (!myImage->isGnssAvailable() || (myImage->isGnssAvailable() && myImage->getGnssType() == "Unused"))
        {
            X0.set(1, 1, X00);
            X0.set(2, 1, Y00);
            X0.set(3, 1, Z00);
        }
        else
        {
            X0.set(1, 1, myImage->getGnssX0());
            X0.set(2, 1, myImage->getGnssY0());
            X0.set(3, 1, myImage->getGnssZ0());
        }
        if (!myImage->isInsAvailable() || (myImage->isInsAvailable() && myImage->getInsType() == "Unused"))
        {
            X0.set(4, 1, phi0);
            X0.set(5, 1, omega0);
            X0.set(6, 1, kappa0);
        }
        else
        {
            X0.set(4, 1, myImage->getInsPhi());
            X0.set(5, 1, myImage->getInsOmega());
            X0.set(6, 1, myImage->getInsKappa());
        }

        generateL0();
    }
}

bool SpatialRessection::calculate(int maxIterations, double gnssPrecision, double insPrecision)
{
    gnssConverged = false;
    insConverged = false;
    if (myImage != NULL && myImage->getSensor() != NULL && myImage->getFlight() != NULL && myImage->getIO() != NULL && (pointForFlightDirectionAvailable || flightDirectionAvailable))
    {
        int iterations = 0;

        Matrix X0temp, L0temp;
        X0temp = X0;
        L0temp = L0;

        if (!(myImage->isGnssAvailable() && myImage->getGnssType() == "Fixed" &&
              myImage->isInsAvailable() && myImage->getInsType() == "Fixed"))
        {
            while (!(gnssConverged && insConverged) && (iterations < maxIterations))
            {

                generateLb();
                generateA();
                generateP();

                Xa = X0 - ((A.transpose() * P * A).inverse() * A.transpose() * P * (L0 - Lb));

                gnssConverged = true;
                if (myImage->isGnssAvailable() && myImage->getGnssType() == "Fixed")
                {
                    Xa.set(1, 1, myImage->getGnssX0());
                    Xa.set(2, 1, myImage->getGnssY0());
                    Xa.set(3, 1, myImage->getGnssZ0());
                }
                else
                {
                    for (int i = 1; i <= 3; i++)
                    {
                        if (std::isinf(fabs(Xa.get(i,1))))
                        {
                            qDebug("Get INF on calculate!");
                            gnssConverged=false;
                            return false;
                        }
                        if (std::isnan(fabs(Xa.get(i,1))))
                        {
                            qDebug("Get NAN on calculate!");
                            gnssConverged=false;
                            return false;
                        }
                        if (fabs(Xa.get(i,1)-X0.get(i,1))/X0.get(i,1)>gnssPrecision)
                            gnssConverged=false;
                    }
                }

                insConverged = true;
                if (myImage->isInsAvailable() && myImage->getInsType() == "Fixed")
                {
                    Xa.set(4, 1, myImage->getInsPhi());
                    Xa.set(5, 1, myImage->getInsOmega());
                    Xa.set(6, 1, myImage->getInsKappa());
                }
                else
                {
                    for (int i = 4; i <= 6; i++)
                    {
                        if (std::isinf(fabs(Xa.get(i,1))))
                        {
                            qDebug("Get INF on calculate!");
                            insConverged=false;
                            return false;
                        }
                        if (std::isnan(fabs(Xa.get(i,1))))
                        {
                            qDebug("Get NAN on calculate!");
                            insConverged=false;
                            return false;
                        }
                        if (fabs(Xa.get(i,1)-X0.get(i,1))/X0.get(i,1)>insPrecision)
                            insConverged=false;
                    }
                }

                generateX0();
                generateL0();
                iterations++;
            }
        }
        else
        {
            generateLb();
            generateA();
            generateP();
            Xa.resize(6,1);
            Xa.set(1, 1, myImage->getGnssX0());
            Xa.set(2, 1, myImage->getGnssY0());
            Xa.set(3, 1, myImage->getGnssZ0());
            Xa.set(4, 1, myImage->getInsPhi());
            Xa.set(5, 1, myImage->getInsOmega());
            Xa.set(6, 1, myImage->getInsKappa());
            gnssConverged = insConverged = true;
        }

        totalIterations = iterations;

        lastL0 = L0;

        X0 = X0temp;
        L0 = L0temp;

        myQuality.calculate(this);
    }
    return gnssConverged && insConverged;
}

// Private support methods
//

DetectorSpaceCoordinate SpatialRessection::applyDistortions(double xi, double eta)
{
    // Por enquanto estou aplicando todas. Crio a flag depois.

    DetectorSpaceCoordinate radial = getRadialDistortions(xi, eta);
    DetectorSpaceCoordinate decentered = getDecenteredDistortions(xi, eta);
    DetectorSpaceCoordinate atmosphere = getAtmosphereDistortions(xi, eta);
    DetectorSpaceCoordinate curvature = getCurvatureDistortions(xi, eta);

    qDebug("%f %f\n%f %f\n%f %f\n%f %f\n\n",radial.getXi(),radial.getEta(),decentered.getXi(),decentered.getEta(),atmosphere.getXi(),atmosphere.getEta(),curvature.getXi(),curvature.getEta());

    double xiFinal = xi + radial.getXi() + decentered.getXi() + atmosphere.getXi() + curvature.getXi();
    double etaFinal = eta + radial.getEta() + decentered.getEta() + atmosphere.getEta() + curvature.getEta();

    DetectorSpaceCoordinate result(myImage->getId());
    result.setXi(xiFinal);
    result.setEta(etaFinal);
    result.setAvailable(true);

    return result;
}

DetectorSpaceCoordinate SpatialRessection::applyDistortions(DetectorSpaceCoordinate myAnalogCoordinate)
{
    return applyDistortions(myAnalogCoordinate.getXi(), myAnalogCoordinate.getEta());
}

DetectorSpaceCoordinate SpatialRessection::removeDistortions(double xi, double eta)
{
    // Por enquanto estou aplicando todas. Crio a flag depois.

    DetectorSpaceCoordinate radial = getRadialDistortions(xi, eta);
    DetectorSpaceCoordinate decentered = getDecenteredDistortions(xi, eta);
    DetectorSpaceCoordinate atmosphere = getAtmosphereDistortions(xi, eta);
    DetectorSpaceCoordinate curvature = getCurvatureDistortions(xi, eta);

    double xiFinal = xi - (radial.getXi() + decentered.getXi() + atmosphere.getXi() + curvature.getXi());
    double etaFinal = eta - (radial.getEta() + decentered.getEta() + atmosphere.getEta() + curvature.getEta());

    DetectorSpaceCoordinate result(myImage->getId());
    result.setXi(xiFinal);
    result.setEta(etaFinal);
    result.setAvailable(true);

    return result;
}

DetectorSpaceCoordinate SpatialRessection::removeDistortions(DetectorSpaceCoordinate myAnalogCoordinate)
{
    return removeDistortions(myAnalogCoordinate.getXi(), myAnalogCoordinate.getEta());
}

DetectorSpaceCoordinate SpatialRessection::getRadialDistortions(double xi, double eta)
{
    DetectorSpaceCoordinate result;
    result.setXi(0.0);
    result.setEta(0.0);
    result.setAvailable(true);

    Sensor* sensor = myImage->getSensor();
    FrameSensor* frame = dynamic_cast<FrameSensor*>(sensor);
    if (frame != NULL)
    {
        double r = sqrt(xi*xi+eta*eta);
        std::deque<RadialSymmetricDistortionCoefficient> rs = frame->getRadialSymmetricCoefficients();
        double dr = 0;

        for (unsigned int i = 0; i < rs.size(); i++)
        {
            dr += rs.at(i).getValue() * pow(r,2*i+1);
        }

        double dx = dr * xi / r;
        double dy = dr * eta / r;

        result.setXi(dx);
        result.setEta(dy);
    }

    return result;
}

DetectorSpaceCoordinate SpatialRessection::getDecenteredDistortions(double xi, double eta)
{
    DetectorSpaceCoordinate result;
    result.setXi(0.0);
    result.setEta(0.0);
    result.setAvailable(true);

    Sensor* sensor = myImage->getSensor();
    FrameSensor* frame = dynamic_cast<FrameSensor*>(sensor);
    if (frame != NULL)
    {
        double r = sqrt(xi*xi+eta*eta);
        std::deque<DecenteredDistortionCoefficient> dec = frame->getDecenteredCoefficients();
        double P1 = dec.at(0).getValue();
        double P2 = dec.at(1).getValue();

        double dx = P1 * (r*r + 2*xi*xi) + 2*P2*xi*eta;
        double dy = 2*P1*xi*eta + P2 * (r*r + 2*eta*eta);

        result.setXi(dx);
        result.setEta(dy);
    }

    return result;
}

DetectorSpaceCoordinate SpatialRessection::getAtmosphereDistortions(double xi, double eta)
{
    DetectorSpaceCoordinate result;
    result.setXi(0.0);
    result.setEta(0.0);
    result.setAvailable(true);

    Flight* flight = myImage->getFlight();
    Sensor* sensor = myImage->getSensor();
    if (flight != NULL && sensor != NULL)
    {
        Terrain* terrain = flight->getTerrain();
        if (terrain != NULL)
        {
            double Z0, Zp;
            /*if (myImage->getEO() != NULL)
   {
    Z0 = myImage->getEO()->getXa().get(3,1);
   }
   else
   {*/
            Z0 = myImage->getFlight()->getScaleDen()*myImage->getSensor()->getFocalDistance()/1000 + terrain->getMeanAltitude();
            /*}*/
            Zp = terrain->getMeanAltitude();

            Z0 /= 1000;
            Zp /= 1000;

            double aux1 = Z0 / ((Z0*Z0) - 6*Z0 + 250);
            double aux2 = Zp / (Z0*((Zp*Zp) - 6*Zp + 250));

            double K = 2410 * 0.000001 * (aux1 - aux2);

            double r = sqrt(xi*xi+eta*eta);
            double f = sensor->getFocalDistance();

            double dr = r * (1 + (r*r/f*f)) * K;

            double dx = dr * xi / r;
            double dy = dr * eta / r;

            result.setXi(dx);
            result.setEta(dy);
        }
    }

    return result;
}

DetectorSpaceCoordinate SpatialRessection::getCurvatureDistortions(double xi, double eta)
{
    DetectorSpaceCoordinate result;
    result.setXi(0.0);
    result.setEta(0.0);
    result.setAvailable(true);

    Flight* flight = myImage->getFlight();
    Sensor* sensor = myImage->getSensor();
    if (flight != NULL && sensor != NULL)
    {
        Terrain* terrain = flight->getTerrain();
        if (terrain != NULL)
        {
            double Z0;
            /*if (myImage->getEO() != NULL)
   {
    Z0 = myImage->getEO()->getXa().get(3,1);
   }
   else
   {*/
            Z0 = myImage->getFlight()->getScaleDen()*myImage->getSensor()->getFocalDistance()/1000 + terrain->getMeanAltitude();
            /*}*/

            Z0 /= 1000;

            double R = 6371004.0; // De onde eu tiro isso? Deixo essa constante?
            double r = sqrt(xi*xi+eta*eta);
            double f = sensor->getFocalDistance();

            double aux = (r*r*Z0)/(2*R*f*f);

            double dx = xi*aux;
            double dy = eta*aux;

            result.setXi(dx);
            result.setEta(dy);
        }
    }

    return result;
}

} // namespace efoto
} // namespace eng
} // namespace uerj
} // namespace br
