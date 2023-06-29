#include "movingBeam.H"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

const double movingBeam::eps = 1e-10;

movingBeam::movingBeam()
    : path( 1, segment() )
    , index_( 0 )
    , power_( 0.0 )

{
    position_.resize( 3, 0.0 );

    // read the scan path file
    readPath();

    // initlialize the path index
    // index_ = findIndex(runTime_.value());

    std::cout << "initial path index: " << index_ << std::endl;
}

void movingBeam::readPath()
{

    const std::string pFile_ = "scanPath.txt";

    std::ifstream is( pFile_ );

    if ( !is.good() )
    {
        std::string error = "Cannot find file " + pFile_;
        throw std::runtime_error( error );
    }
    else
    {
        std::cout << "Reading scan path from: " << pFile_ << std::endl;
    }

    std::string line;

    // skip the header line
    std::getline( is, line );

    while ( std::getline( is, line ) )
    {
        if ( line.empty() )
        {
            continue;
        }

        path.push_back( segment( line ) );
    }

    for ( int i = 1; i < path.size(); i++ )
    {
        if ( path[i].mode() == 1 )
        {
            path[i].setTime( path[i - 1].time() + path[i].parameter() );
        }
        else
        {
            std::vector<double> p0 = path[i - 1].position();
            std::vector<double> p1 = path[i].position();

            double d_ = sqrt( ( p0[0] - p1[0] ) * ( p0[0] - p1[0] ) +
                              ( p0[1] - p1[1] ) * ( p0[1] - p1[1] ) +
                              ( p0[2] - p1[2] ) * ( p0[2] - p1[2] ) );

            path[i].setTime( path[i - 1].time() + d_ / path[i].parameter() );
        }

        // std::cout << i << "\t" << path[i].time() << "\n";
    }
}

void movingBeam::move( const double time )
{
    // update the current index of the path
    index_ = findIndex( time );

    const int i = index_;

    // update the beam center
    if ( path[i].mode() == 1 )
    {
        position_ = path[i].position();
    }
    else
    {
        std::vector<double> displacement( 3, 0 );

        double dt = path[i].time() - path[i - 1].time();

        if ( dt > 0 )
        {
            std::vector<double> dx( 3, 0 );
            dx[0] = path[i].position()[0] - path[i - 1].position()[0];
            dx[1] = path[i].position()[1] - path[i - 1].position()[1];
            dx[2] = path[i].position()[2] - path[i - 1].position()[2];
            displacement[0] = dx[0] * ( time - path[i - 1].time() ) / dt;
            displacement[1] = dx[1] * ( time - path[i - 1].time() ) / dt;
            displacement[2] = dx[2] * ( time - path[i - 1].time() ) / dt;
        }

        position_[0] = path[i - 1].position()[0] + displacement[0];
        position_[1] = path[i - 1].position()[1] + displacement[1];
        position_[2] = path[i - 1].position()[2] + displacement[2];
    }

    // update the beam power
    if ( ( time - path[i - 1].time() ) > eps )
    {
        power_ = path[i].power();
    }
    else
    {
        power_ = path[i - 1].power();
    }

    // std::cout << "movingBeam position: "
    //<< position_[0] << " " << position_[1] << " " << position_[2] << "\t" <<
    //"power: " << power_ << std::endl;
}

int movingBeam::findIndex( const double time )
{
    int i = index_;

    const int n = path.size() - 1;

    // step back path index for safe updating
    for ( i = i; i > 0 && path[i].time() > time; --i )
    {
    }

    // update the path index to the provided time
    for ( i = i; i < n && path[i].time() < time; ++i )
    {
    }

    // skip any point sources with zero time
    while ( i < n )
    {
        if ( path[i].mode() == 1 && path[i].parameter() == 0 )
        {
            ++i;
        }
        else
        {
            break;
        }
    }

    return std::min( std::max( i, 0 ), n );
}

//
// bool movingBeam::activePath()
//{
//    return ((endTime_ - runTime_.value()) > eps);
//}
//
//
// void movingBeam::adjustDeltaT(scalar& dt)
//{
//    if (activePath())
//    {
//        // reset time step to initial value if the beam turns on
//        if (power_ < small)
//        {
//            scalar nextTime = runTime_.userTimeToTime(runTime_.value() + dt);
//
//            scalar nextPower = path[findIndex(nextTime)].power();
//
//            if (nextPower > small)
//            {
//                scalar dt0 =
//                    runTime_.userTimeToTime
//                    (
//                        runTime_.controlDict().lookup<scalar>("deltaT")
//                    );
//
//                dt = min(dt, dt0);
//            }
//        }
//
//        // adjust time step to hit next path start time
//        if (hitPathIntervals_)
//        {
//            scalar timeToNextPath = 0;
//            label i = index_;
//
//            while (timeToNextPath < eps)
//            {
//                timeToNextPath = max(0, path[i].time() - runTime_.value());
//
//                i++;
//
//                if (i == path.size())
//                {
//                    break;
//                }
//            }
//
//            const scalar nSteps = timeToNextPath/dt;
//
//            if (nSteps < labelMax)
//            {
//                // allow time step to dilate 1% to hit target path time
//                const label nStepsToNextPath = label(max(nSteps, 1) + 0.99);
//                dt = min(timeToNextPath/nStepsToNextPath, dt);
//            }
//        }
//    }
//}

// ************************************************************************* //
