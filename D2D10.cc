/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * NIST-developed software is provided by NIST as a public
 * service. You may use, copy and distribute copies of the software in
 * any medium, provided that you keep intact this entire notice. You
 * may improve, modify and create derivative works of the software or
 * any portion of the software, and you may copy and distribute such
 * modifications or works. Modified works should carry a notice
 * stating that you changed the software and should note the date and
 * nature of any such change. Please explicitly acknowledge the
 * National Institute of Standards and Technology as the source of the
 * software.
 *
 * NIST-developed software is expressly provided "AS IS." NIST MAKES
 * NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY
 * OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR
 * WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED
 * OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT
 * WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE
 * SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE
 * CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
 *
 * You are solely responsible for determining the appropriateness of
 * using and distributing the software and you assume all risks
 * associated with its use, including but not limited to the risks and
 * costs of program errors, compliance with applicable laws, damage to
 * or loss of data, programs or equipment, and the unavailability or
 * interruption of operation. This software is not intended to be used
 * in any situation where a failure could cause risk of injury or
 * damage to property. The software developed by NIST employees is not
 * subject to copyright protection within the United States.
 */


#include "ns3/lte-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/config-store.h"
#include <ns3/lte-ue-phy.h>
#include <cfloat>
#include <sstream>
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <bits/stdc++.h>

using namespace ns3;

// This trace will log packet transmissions and receptions from the application
// layer.  The parameter 'localAddrs' is passed to this trace in case the
// address passed by the trace is not set (i.e., is '0.0.0.0' or '::').  The
// trace writes to a file stream provided by the first argument; by default,
// this trace file is 'UePacketTrace.tr'
void
UePacketTrace (Ptr<OutputStreamWrapper> stream, const Address &localAddrs, std::string context, Ptr<const Packet> p, const Address &srcAddrs, const Address &dstAddrs)
{
  std::ostringstream oss;
  *stream->GetStream () << Simulator::Now ().GetNanoSeconds () / (double) 1e9 << "\t" << context << "\t" << p->GetSize () << "\t";
  if (InetSocketAddress::IsMatchingType (srcAddrs))
    {
      oss << InetSocketAddress::ConvertFrom (srcAddrs).GetIpv4 ();
      if (!oss.str ().compare ("0.0.0.0")) //srcAddrs not set
        {
          *stream->GetStream () << Ipv4Address::ConvertFrom (localAddrs) << ":" << InetSocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" << InetSocketAddress::ConvertFrom (dstAddrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
        }
      else
        {
          oss.str ("");
          oss << InetSocketAddress::ConvertFrom (dstAddrs).GetIpv4 ();
          if (!oss.str ().compare ("0.0.0.0")) //dstAddrs not set
            {
              *stream->GetStream () << InetSocketAddress::ConvertFrom (srcAddrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" <<  Ipv4Address::ConvertFrom (localAddrs) << ":" << InetSocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
            }
          else
            {
              *stream->GetStream () << InetSocketAddress::ConvertFrom (srcAddrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" << InetSocketAddress::ConvertFrom (dstAddrs).GetIpv4 () << ":" << InetSocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
            }
        }
    }
  else if (Inet6SocketAddress::IsMatchingType (srcAddrs))
    {
      oss << Inet6SocketAddress::ConvertFrom (srcAddrs).GetIpv6 ();
      if (!oss.str ().compare ("::")) //srcAddrs not set
        {
          *stream->GetStream () << Ipv6Address::ConvertFrom (localAddrs) << ":" << Inet6SocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" << Inet6SocketAddress::ConvertFrom (dstAddrs).GetIpv6 () << ":" << Inet6SocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
        }
      else
        {
          oss.str ("");
          oss << Inet6SocketAddress::ConvertFrom (dstAddrs).GetIpv6 ();
          if (!oss.str ().compare ("::")) //dstAddrs not set
            {
              *stream->GetStream () << Inet6SocketAddress::ConvertFrom (srcAddrs).GetIpv6 () << ":" << Inet6SocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" << Ipv6Address::ConvertFrom (localAddrs) << ":" << Inet6SocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
            }
          else
            {
              *stream->GetStream () << Inet6SocketAddress::ConvertFrom (srcAddrs).GetIpv6 () << ":" << Inet6SocketAddress::ConvertFrom (srcAddrs).GetPort () << "\t" << Inet6SocketAddress::ConvertFrom (dstAddrs).GetIpv6 () << ":" << Inet6SocketAddress::ConvertFrom (dstAddrs).GetPort () << std::endl;
            }
        }
    }
  else
    {
      *stream->GetStream () << "Unknown address type!" << std::endl;
    }
}


 
 // class for d2d device
 class d2d_user {
        public :
        
        NodeContainer d2dNodes;
        int Node_id ;
        int application_type;
        int cluster_id;
        Vector position;
        int count = -1;
        Ipv4Address address1, address2;
        
        void store(Vector ue1, Vector ue2, int id) {
                Node_id = id;
                application_type = rand()%2+1;
                position.x = (ue1.x + ue2.x)/2;
                position.y = (ue1.y + ue2.y)/2;
                position.z = (ue1.z + ue2.z)/2;
        }
 };
 
 // class for cu device
 class cu_users {
        public :
        
        NodeContainer Node;
        int application_type;
        int cluster_id;
        Vector position;
        Ipv4Address address;
        int occupied = 0;
        
        void store(Vector cu) {
                position.x = cu.x;
                position.y = cu.y;
                position.z = cu.z;
        }
 };
 
 // class for center
 struct point {
	double x, y;
	int cluster = -1;
	int application;
 };
 
 
 int shard1[10], shard2[10], shard3[10];
 int size1=0, size2=0, size3=0;
  
  
  
  // k means algorithm
  std::vector<point> k_means(std::vector<d2d_user> init, int k, d2d_user* ptr)
   {
    auto tmp = std::max_element(init.begin(), init.end(), [](d2d_user a, d2d_user b) {return a.position.x < b.position.x; });
    int max_x = tmp->position.x;
    tmp = std::max_element(init.begin(), init.end(), [](d2d_user a, d2d_user b) {return a.position.y < b.position.y; });
    int max_y = tmp->position.y;
    tmp = std::min_element(init.begin(), init.end(), [](d2d_user a, d2d_user b) {return a.position.x < b.position.x; });
    int min_x = tmp->position.x;
    tmp = std::min_element(init.begin(), init.end(), [](d2d_user a, d2d_user b) {return a.position.y < b.position.y; });
    int min_y = tmp->position.y;
    
    std::vector<point> centers(k);
    for (int i = 0; i < k; i++)
    {
        centers[i].x = (rand() + i*23) % (max_x - min_x + 1) + min_x;
	centers[i].y = (rand() - i*17) % (max_y - min_y + 1) + min_y;
	centers[i].cluster = i;
    }
		
    for (int i = 0; i < 1000; i++)
     {
	for (long unsigned int j = 0; j < init.size(); j++)
	{
		double* dists = new double[k];
		for (int p = 0; p < k; p++)
		{
			double a = std::abs(init[j].position.y - centers[p].y);	
			double b = std::abs(init[j].position.x - centers[p].x);	
			dists[p] = std::sqrt(std::pow(a, 2) + std::pow(b, 2));	
		}
		init[j].cluster_id = std::min_element(dists, dists + k) - dists;
		ptr[init[j].Node_id].cluster_id = init[j].cluster_id;
		delete[] dists;
	}
		
	std::unique_ptr<double[]> sum_x(new double[k], std::default_delete<double[]>());
	std::unique_ptr<double[]> sum_y(new double[k], std::default_delete<double[]>());
	int count[k];
	
	for (int p = 0; p < k; p++)
	{
		sum_x[p] = 0;
		sum_y[p] = 0;
		count[p] = 0;
	}
	for (long unsigned int f = 0; f < init.size(); f++)
	{
		sum_x[init[f].cluster_id] += init[f].position.x;
		sum_y[init[f].cluster_id] += init[f].position.y;
		count[init[f].cluster_id]++;
		ptr[init[f].Node_id].count = count[init[f].cluster_id];
	}

	for (int f = 0; f < k; f++)
	{
		centers[f].x = sum_x[f] / count[f];
		centers[f].y = sum_y[f] / count[f];
	}
      }
      
    return centers;
   }
   
   
   //print the shards
   void print(d2d_user d2d[], cu_users cu[])
   {
     std::cout<<"\n";
     for(int i=0; i<60 ; i++) std::cout<<"*";
     std::cout<<"\n";
     std::cout<< "                            SOCIAL_MEDIA"<<std::endl;
     std::cout<<"\n";
     std::cout<< " Shard 1 = { "<<std::endl;
     for(int i=0;i<5;i++)
     {
        if(d2d[i].application_type == 1)
        {
                if(d2d[i].cluster_id == 0 )
                {
                        std::cout<< "                 "<<"UE"<<2*i<<" "<<" , IP_Address : "<<d2d[i].address1<<std::endl;
                        std::cout<< "                 "<<"UE"<<2*i+1<<" "<<" , IP_Address : "<<d2d[i].address2<<std::endl;
                }
        }
     }
     for(int i=0;i<3;i++)
     {
        if(cu[i].application_type == 1)
        {
                if(cu[i].cluster_id == 0 )
                {
                        std::cout<< "                 "<<"CU"<<i+1<<"  , IP_Address : "<<cu[i].address<<std::endl;
                }
        }
     }
     std::cout<< "           } "<<std::endl<<std::endl;
     for(int i=0; i<60 ; i++) std::cout<<"*";
     std::cout<<"\n";
      
     std::cout<< "                       PUBLIC_SAFETY "<<std::endl;
     std::cout<<"\n";
     std::cout<<" Shard 1 = { " <<std::endl; 
     for(int i=0;i<5;i++)
     {
        if(d2d[i].application_type == 2)
        {
                if(d2d[i].cluster_id == 0 )
                {
                      std::cout<< "                 "<<"UE"<<2*i<<" "<<" , IP_Address : "<<d2d[i].address1<<std::endl;
                      std::cout<< "                 "<<"UE"<<2*i+1<<" "<<" , IP_Address : "<<d2d[i].address2<<std::endl;
                }
        }
     }
      for(int i=0;i<3;i++)
      {
        if(cu[i].application_type == 2)
        {
                if(cu[i].cluster_id == 0)
                {
                      std::cout<< "                 "<<"CU"<<i+1<<"  , IP_Address : "<<cu[i].address<<std::endl;
                }
        }
     }
      std::cout<< "           } "<<std::endl<<std::endl;
      
      std::cout<<"\n";
      std::cout<<" Shard 2 = { " <<std::endl; 
      for(int i=0;i<5;i++)
      {
        if(d2d[i].application_type == 2)
        {
                if(d2d[i].cluster_id == 1 )
                {
                      std::cout<< "                 "<<"UE"<<2*i<<" "<<" , IP_Address : "<<d2d[i].address1<<std::endl;
                      std::cout<< "                 "<<"UE"<<2*i+1<<" "<<" , IP_Address : "<<d2d[i].address2<<std::endl;
                }
        }
     }
     for(int i=0;i<3;i++)
     {
        if(cu[i].application_type == 2)
        {
                if(cu[i].cluster_id == 1 )
                {
                        std::cout<< "                 "<<"CU"<<i+1<<"  , IP_Address : "<<cu[i].address<<std::endl;
                }
        }
     }
     std::cout<< "            } "<<std::endl<<std::endl;
     for(int i=0; i<60 ; i++) std::cout<<"*";
     std::cout<<"\n";
   }
  
 

NS_LOG_COMPONENT_DEFINE ("LteSlInCovrgCommMode1");


// function to print the SINR values
//print SINR value of first shard

void PhySnirTrace1 (std::string context ,uint16_t cellId, uint16_t rnti, double rsrp, double sinr, uint8_t componentCarrierId)
{ 
    

    if(Simulator::Now().GetSeconds() >= 2.5 && Simulator::Now().GetSeconds() <= 2.50100 )
    { 
        int nth=3;  //looking for the second ocurrence of "/"
        int cnt=0; 
        size_t pos=0;

        while( cnt != nth )
        {
                pos = context.find("/", pos);
                if ( pos == std::string::npos )
                        std::cout << nth << "th ocurrence not found!"<< std::endl;
                pos+=1;
                cnt++;
        }

        std::string str1;
        str1 = context.substr(10,pos-11);
        int ue= atoi(str1.c_str())-7;
    
    
        for(int i=0; i< size1; i++) 
        {
                if(2*shard1[i] == ue && i == 0)
                { 
                        std::cout<<" UE"<<ue<<" belonging to D2D"<<i+1<<"\n";
                        std::cout<<" Time : " <<Simulator::Now().GetSeconds()<<"\n";
                        std::cout<<" RNTI : "<<rnti<<"                 rsrp : "<<rsrp<<"               SINR : "<<sinr<<std::endl;
                        std::cout<<"\n";
                }
        }
     //std::cout<<" Time : " <<Simulator::Now().GetSeconds()<<"\n";
     
    }
    
    if(Simulator::Now().GetSeconds() >= 3.5 && Simulator::Now().GetSeconds() <= 3.50100 )
    {
    
        int nth=3;  //looking for the second ocurrence of "/"
        int cnt=0; 
        size_t pos=0;

        while( cnt != nth )
        {
                pos = context.find("/", pos);
                if ( pos == std::string::npos )
                std::cout << nth << "th ocurrence not found!"<< std::endl;
                pos+=1;
                cnt++;
        }

        std::string str1;
        str1 = context.substr(10,pos-11);
        int ue= atoi(str1.c_str())-7;
        
        for(int i=0; i< size1; i++) 
        {
                if(2*shard1[i] == ue && (i == 0 || i == 1) )
                { 
                        //std::cout<<context<<std::endl;
                        std::cout<<" UE"<<ue<<" belonging to D2D"<<i+1<<"\n";
                        std::cout<<" Time : " <<Simulator::Now().GetSeconds()<<"\n";
                        std::cout<<" RNTI : "<<rnti<<"                 rsrp : "<<rsrp<<"               SINR : "<<sinr<<std::endl;
                        std::cout<<"\n";
                }
        }
    }
    
    if(Simulator::Now().GetSeconds() >= 4.5 && Simulator::Now().GetSeconds() <= 4.50100)
    {
    
        int nth=3;  //looking for the second ocurrence of "/"
        int cnt=0; 
        size_t pos=0;

        while( cnt != nth )
        {
                pos = context.find("/", pos);
                if ( pos == std::string::npos )
                        std::cout << nth << "th ocurrence not found!"<< std::endl;
                pos+=1;
                cnt++;
        }

        std::string str1;
        str1 = context.substr(10,pos-11);
        int ue= atoi(str1.c_str())-7;
    
    
        for(int i=0; i< size1; i++) 
        {
                if(2*shard1[i] == ue && (i == 0 || i == 1 || i == 2))
                { 
                        //std::cout<<context<<std::endl;
                        std::cout<<" UE"<<ue<<" belonging to D2D"<<i+1<<"\n";
                        std::cout<<" Time : " <<Simulator::Now().GetSeconds()<<"\n";
                        std::cout<<" RNTI : "<<rnti<<"                 rsrp : "<<rsrp<<"               SINR : "<<sinr<<std::endl;
                        std::cout<<"\n";
                }
        }
     
    }
    
    if(Simulator::Now().GetSeconds() >= 5.5 && Simulator::Now().GetSeconds() <= 5.50100)
    {
    
        int nth=3;  //looking for the second ocurrence of "/"
        int cnt=0; 
        size_t pos=0;

        while( cnt != nth )
        {
                pos = context.find("/", pos);
                if ( pos == std::string::npos )
                        std::cout << nth << "th ocurrence not found!"<< std::endl;
                pos+=1;
                cnt++;
        }

        std::string str1;
        str1 = context.substr(10,pos-11);
        int ue= atoi(str1.c_str())-7;
        
        for(int i=0; i< size1; i++) 
        {
                if(2*shard1[i] == ue && (i == 0 || i == 1 || i == 2))
                { 
                        //std::cout<<context<<std::endl;
                        std::cout<<" UE"<<ue<<" belonging to D2D"<<i+1<<"\n";
                        std::cout<<" Time : " <<Simulator::Now().GetSeconds()<<"\n";
                        std::cout<<" RNTI : "<<rnti<<"                 rsrp : "<<rsrp<<"               SINR : "<<sinr<<std::endl;
                        std::cout<<"\n";
                }
        }
     
    }
}

// printing the SINR value of second shard

void PhySnirTrace2 (std::string context ,uint16_t cellId, uint16_t rnti, double rsrp, double sinr, uint8_t componentCarrierId)
{ 
    

    if(Simulator::Now().GetSeconds() >= 2.5 && Simulator::Now().GetSeconds() <= 2.50100 )
    { 
        int nth=3;  //looking for the second ocurrence of "/"
        int cnt=0; 
        size_t pos=0;

        while( cnt != nth )
        {
                pos = context.find("/", pos);
                if ( pos == std::string::npos )
                        std::cout << nth << "th ocurrence not found!"<< std::endl;
                pos+=1;
                cnt++;
        }

        std::string str1;
        str1 = context.substr(10,pos-11);
        int ue= atoi(str1.c_str())-7;
    
    
        for(int i=0; i< size2; i++) 
        {
                if(2*shard2[i] == ue && i == 0)
                { 
                        std::cout<<" UE"<<ue<<" belonging to D2D"<<i+1<<"\n";
                        std::cout<<" Time : " <<Simulator::Now().GetSeconds()<<"\n";
                        std::cout<<" RNTI : "<<rnti<<"                 rsrp : "<<rsrp<<"               SINR : "<<sinr<<std::endl;
                        std::cout<<"\n";
                }
        }
     //std::cout<<" Time : " <<Simulator::Now().GetSeconds()<<"\n";
     
    }
    
    if(Simulator::Now().GetSeconds() >= 3.5 && Simulator::Now().GetSeconds() <= 3.50100 )
    {
    
        int nth=3;  //looking for the second ocurrence of "/"
        int cnt=0; 
        size_t pos=0;

        while( cnt != nth )
        {
                pos = context.find("/", pos);
                if ( pos == std::string::npos )
                std::cout << nth << "th ocurrence not found!"<< std::endl;
                pos+=1;
                cnt++;
        }

        std::string str1;
        str1 = context.substr(10,pos-11);
        int ue= atoi(str1.c_str())-7;
        
        for(int i=0; i< size2; i++) 
        {
                if(2*shard2[i] == ue && (i == 0 || i == 1) )
                { 
                        //std::cout<<context<<std::endl;
                        std::cout<<" UE"<<ue<<" belonging to D2D"<<i+1<<"\n";
                        std::cout<<" Time : " <<Simulator::Now().GetSeconds()<<"\n";
                        std::cout<<" RNTI : "<<rnti<<"                 rsrp : "<<rsrp<<"               SINR : "<<sinr<<std::endl;
                        std::cout<<"\n";
                }
        }
    }
    
    if(Simulator::Now().GetSeconds() >= 4.5 && Simulator::Now().GetSeconds() <= 4.50100)
    {
    
        int nth=3;  //looking for the second ocurrence of "/"
        int cnt=0; 
        size_t pos=0;

        while( cnt != nth )
        {
                pos = context.find("/", pos);
                if ( pos == std::string::npos )
                        std::cout << nth << "th ocurrence not found!"<< std::endl;
                pos+=1;
                cnt++;
        }

        std::string str1;
        str1 = context.substr(10,pos-11);
        int ue= atoi(str1.c_str())-7;
    
    
        for(int i=0; i< size2; i++) 
        {
                if(2*shard2[i] == ue && (i == 0 || i == 1 || i == 2))
                { 
                        //std::cout<<context<<std::endl;
                        std::cout<<" UE"<<ue<<" belonging to D2D"<<i+1<<"\n";
                        std::cout<<" Time : " <<Simulator::Now().GetSeconds()<<"\n";
                        std::cout<<" RNTI : "<<rnti<<"                 rsrp : "<<rsrp<<"               SINR : "<<sinr<<std::endl;
                        std::cout<<"\n";
                }
        }
     
    }
    
    if(Simulator::Now().GetSeconds() >= 5.5 && Simulator::Now().GetSeconds() <= 5.50100)
    {
    
        int nth=3;  //looking for the second ocurrence of "/"
        int cnt=0; 
        size_t pos=0;

        while( cnt != nth )
        {
                pos = context.find("/", pos);
                if ( pos == std::string::npos )
                        std::cout << nth << "th ocurrence not found!"<< std::endl;
                pos+=1;
                cnt++;
        }

        std::string str1;
        str1 = context.substr(10,pos-11);
        int ue= atoi(str1.c_str())-7;
        
        for(int i=0; i< size2; i++) 
        {
                if(2*shard2[i] == ue && (i == 0 || i == 1 || i == 2))
                { 
                        //std::cout<<context<<std::endl;
                        std::cout<<" UE"<<ue<<" belonging to D2D"<<i+1<<"\n";
                        std::cout<<" Time : " <<Simulator::Now().GetSeconds()<<"\n";
                        std::cout<<" RNTI : "<<rnti<<"                 rsrp : "<<rsrp<<"               SINR : "<<sinr<<std::endl;
                        std::cout<<"\n";
                }
        }
     
    }
}

// print SINR value of third shard


void PhySnirTrace3 (std::string context ,uint16_t cellId, uint16_t rnti, double rsrp, double sinr, uint8_t componentCarrierId)
{ 
    

    if(Simulator::Now().GetSeconds() >= 2.5 && Simulator::Now().GetSeconds() <= 2.50100 )
    { 
        int nth=3;  //looking for the second ocurrence of "/"
        int cnt=0; 
        size_t pos=0;

        while( cnt != nth )
        {
                pos = context.find("/", pos);
                if ( pos == std::string::npos )
                        std::cout << nth << "th ocurrence not found!"<< std::endl;
                pos+=1;
                cnt++;
        }

        std::string str1;
        str1 = context.substr(10,pos-11);
        int ue= atoi(str1.c_str())-7;
    
    
        for(int i=0; i< size3; i++) 
        {
                if(2*shard3[i] == ue && i == 0)
                { 
                        std::cout<<" UE"<<ue<<" belonging to D2D"<<i+1<<"\n";
                        std::cout<<" Time : " <<Simulator::Now().GetSeconds()<<"\n";
                        std::cout<<" RNTI : "<<rnti<<"                 rsrp : "<<rsrp<<"               SINR : "<<sinr<<std::endl;
                        std::cout<<"\n";
                }
        }
     //std::cout<<" Time : " <<Simulator::Now().GetSeconds()<<"\n";
     
    }
    
    if(Simulator::Now().GetSeconds() >= 3.5 && Simulator::Now().GetSeconds() <= 3.50100 )
    {
    
        int nth=3;  //looking for the second ocurrence of "/"
        int cnt=0; 
        size_t pos=0;

        while( cnt != nth )
        {
                pos = context.find("/", pos);
                if ( pos == std::string::npos )
                std::cout << nth << "th ocurrence not found!"<< std::endl;
                pos+=1;
                cnt++;
        }

        std::string str1;
        str1 = context.substr(10,pos-11);
        int ue= atoi(str1.c_str())-7;
        
        for(int i=0; i< size3; i++) 
        {
                if(2*shard3[i] == ue && (i == 0 || i == 1) )
                { 
                        //std::cout<<context<<std::endl;
                        std::cout<<" UE"<<ue<<" belonging to D2D"<<i+1<<"\n";
                        std::cout<<" Time : " <<Simulator::Now().GetSeconds()<<"\n";
                        std::cout<<" RNTI : "<<rnti<<"                 rsrp : "<<rsrp<<"               SINR : "<<sinr<<std::endl;
                        std::cout<<"\n";
                }
        }
    }
    
    if(Simulator::Now().GetSeconds() >= 4.5 && Simulator::Now().GetSeconds() <= 4.50100)
    {
    
        int nth=3;  //looking for the second ocurrence of "/"
        int cnt=0; 
        size_t pos=0;

        while( cnt != nth )
        {
                pos = context.find("/", pos);
                if ( pos == std::string::npos )
                        std::cout << nth << "th ocurrence not found!"<< std::endl;
                pos+=1;
                cnt++;
        }

        std::string str1;
        str1 = context.substr(10,pos-11);
        int ue= atoi(str1.c_str())-7;
    
    
        for(int i=0; i< size3; i++) 
        {
                if(2*shard3[i] == ue && (i == 0 || i == 1 || i == 2))
                { 
                        //std::cout<<context<<std::endl;
                        std::cout<<" UE"<<ue<<" belonging to D2D"<<i+1<<"\n";
                        std::cout<<" Time : " <<Simulator::Now().GetSeconds()<<"\n";
                        std::cout<<" RNTI : "<<rnti<<"                 rsrp : "<<rsrp<<"               SINR : "<<sinr<<std::endl;
                        std::cout<<"\n";
                }
        }
     
    }
    
    if(Simulator::Now().GetSeconds() >= 5.5 && Simulator::Now().GetSeconds() <= 5.50100)
    {
    
        int nth=3;  //looking for the second ocurrence of "/"
        int cnt=0; 
        size_t pos=0;

        while( cnt != nth )
        {
                pos = context.find("/", pos);
                if ( pos == std::string::npos )
                        std::cout << nth << "th ocurrence not found!"<< std::endl;
                pos+=1;
                cnt++;
        }

        std::string str1;
        str1 = context.substr(10,pos-11);
        int ue= atoi(str1.c_str())-7;
        
        for(int i=0; i< size3; i++) 
        {
                if(2*shard3[i] == ue && (i == 0 || i == 1 || i == 2))
                { 
                        //std::cout<<context<<std::endl;
                        std::cout<<" UE"<<ue<<" belonging to D2D"<<i+1<<"\n";
                        std::cout<<" Time : " <<Simulator::Now().GetSeconds()<<"\n";
                        std::cout<<" RNTI : "<<rnti<<"                 rsrp : "<<rsrp<<"               SINR : "<<sinr<<std::endl;
                        std::cout<<"\n";
                }
        }
     
    }
}





int main (int argc, char *argv[])
{
  Time simTime = Seconds (6);
  bool enableNsLogs = false;
  bool useIPv6 = false;

  CommandLine cmd;
  cmd.AddValue ("simTime", "Total duration of the simulation", simTime);
  cmd.AddValue ("enableNsLogs", "Enable ns-3 logging (debug builds)", enableNsLogs);
  cmd.AddValue ("useIPv6", "Use IPv6 instead of IPv4", useIPv6);
  cmd.Parse (argc, argv);

  // Configure the scheduler
  Config::SetDefault ("ns3::RrSlFfMacScheduler::Itrp", UintegerValue (0));
  //The number of RBs allocated per UE for Sidelink
  Config::SetDefault ("ns3::RrSlFfMacScheduler::SlGrantSize", UintegerValue (5));

  //Set the frequency

  Config::SetDefault ("ns3::LteEnbNetDevice::DlEarfcn", UintegerValue (100));
  Config::SetDefault ("ns3::LteUeNetDevice::DlEarfcn", UintegerValue (100));
  Config::SetDefault ("ns3::LteEnbNetDevice::UlEarfcn", UintegerValue (18100));
  Config::SetDefault ("ns3::LteEnbNetDevice::DlBandwidth", UintegerValue (50));
  Config::SetDefault ("ns3::LteEnbNetDevice::UlBandwidth", UintegerValue (50));

  // Set error models
  Config::SetDefault ("ns3::LteSpectrumPhy::SlCtrlErrorModelEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::LteSpectrumPhy::SlDataErrorModelEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::LteSpectrumPhy::DropRbOnCollisionEnabled", BooleanValue (false));

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();
  // parse again so we can override input file default values via command line
  cmd.Parse (argc, argv);

  if (enableNsLogs)
    {
      LogLevel logLevel = (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_LEVEL_ALL);

      LogComponentEnable ("LteUeRrc", logLevel);
      LogComponentEnable ("LteUeMac", logLevel);
      LogComponentEnable ("LteSpectrumPhy", logLevel);
      LogComponentEnable ("LteUePhy", logLevel);
      LogComponentEnable ("LteEnbPhy", logLevel);
    }

  //Set the UEs power in dBm
  Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (10.0));
  //Set the eNBs power in dBm
  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (30.0));

  //Sidelink bearers activation time
  Time slBearersActivationTime = Seconds (2.0);

  //Create the helpers
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  //Create and set the EPC helper
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  ////Create Sidelink helper and set lteHelper
  Ptr<LteSidelinkHelper> proseHelper = CreateObject<LteSidelinkHelper> ();
  proseHelper->SetLteHelper (lteHelper);

  //Set pathloss model
  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::Cost231PropagationLossModel"));

  //Enable Sidelink
  lteHelper->SetAttribute ("UseSidelink", BooleanValue (true));

  //Sidelink Round Robin scheduler
  lteHelper->SetSchedulerType ("ns3::RrSlFfMacScheduler");

  //Create nodes (eNb + UEs)
  NodeContainer enbNode;
  enbNode.Create (1);
  NS_LOG_INFO ("eNb node id = [" << enbNode.Get (0)->GetId () << "]");
  
  //d2d devices
  d2d_user d2d[10];
  for(int i=0 ; i<5; i++) d2d[i].d2dNodes.Create (2);
    
  //CU devices
  cu_users cu[10];
  for(int i=0; i<3; i++) cu[i].Node.Create(1);
    
  NS_LOG_INFO ("UE 1 node id = [" << d2d[1].d2dNodes.Get (0)->GetId () << "]");
  NS_LOG_INFO ("UE 2 node id = [" << d2d[1].d2dNodes.Get (1)->GetId () << "]");
  

  //Position of the nodes
  Ptr<ListPositionAllocator> positionAllocEnb = CreateObject<ListPositionAllocator> ();
  positionAllocEnb->Add (Vector (10.0, 0.0, 0.0));
  
  //FOR CU
  Ptr<ListPositionAllocator> positionAllocCu1 = CreateObject<ListPositionAllocator> ();
  positionAllocCu1->Add (Vector (5.0, 5.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocCu2 = CreateObject<ListPositionAllocator> ();
  positionAllocCu2->Add (Vector (15.0, 5.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocCu3 = CreateObject<ListPositionAllocator> ();
  positionAllocCu3->Add (Vector (10.0, 2.0, 0.0));
  
  //FOR UE
  Ptr<ListPositionAllocator> positionAllocd2d1 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d1->Add (Vector (5.0, 6.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d2 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d2->Add (Vector (6.0, 6.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d3 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d3->Add (Vector (3.0, 3.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d4 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d4->Add (Vector (4.0, 3.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d5 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d5->Add (Vector (16.0, 5.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d6 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d6->Add (Vector (17.0, 5.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d7 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d7->Add (Vector (22.0, -4.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d8 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d8->Add (Vector (24.0, -4.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d9 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d9->Add (Vector (-23.0, 7.0, 0.0));
  Ptr<ListPositionAllocator> positionAllocd2d10 = CreateObject<ListPositionAllocator> ();
  positionAllocd2d10->Add (Vector (-19.0, 7.0, 0.0));








  //Install mobility
  MobilityHelper mobilityeNodeB;
  mobilityeNodeB.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityeNodeB.SetPositionAllocator (positionAllocEnb);
  mobilityeNodeB.Install (enbNode);
  
  MobilityHelper mobilityCu1;
  mobilityCu1.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityCu1.SetPositionAllocator (positionAllocCu1);
  mobilityCu1.Install (cu[0].Node.Get (0));
   
  MobilityHelper mobilityCu2;
  mobilityCu2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityCu2.SetPositionAllocator (positionAllocCu2);
  mobilityCu2.Install (cu[1].Node.Get (0));
  
  MobilityHelper mobilityCu3;
  mobilityCu3.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityCu3.SetPositionAllocator (positionAllocCu3);
  mobilityCu3.Install (cu[2].Node.Get (0));

  MobilityHelper mobilityd2d1;
  mobilityd2d1.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d1.SetPositionAllocator (positionAllocd2d1);
  mobilityd2d1.Install (d2d[0].d2dNodes.Get (0));
  
  MobilityHelper mobilityd2d2;
  mobilityd2d2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d2.SetPositionAllocator (positionAllocd2d2);
  mobilityd2d2.Install (d2d[0].d2dNodes.Get (1));
  
  MobilityHelper mobilityd2d3;
  mobilityd2d3.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d3.SetPositionAllocator (positionAllocd2d3);
  mobilityd2d3.Install (d2d[1].d2dNodes.Get (0));

  MobilityHelper mobilityd2d4;
  mobilityd2d4.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d4.SetPositionAllocator (positionAllocd2d4);
  mobilityd2d4.Install (d2d[1].d2dNodes.Get (1));
  
  MobilityHelper mobilityd2d5;
  mobilityd2d5.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d5.SetPositionAllocator (positionAllocd2d5);
  mobilityd2d5.Install (d2d[2].d2dNodes.Get (0));

  MobilityHelper mobilityd2d6;
  mobilityd2d6.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d6.SetPositionAllocator (positionAllocd2d6);
  mobilityd2d6.Install (d2d[2].d2dNodes.Get (1));
  
  MobilityHelper mobilityd2d7;
  mobilityd2d7.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d7.SetPositionAllocator (positionAllocd2d7);
  mobilityd2d7.Install (d2d[3].d2dNodes.Get (0));
  
  MobilityHelper mobilityd2d8;
  mobilityd2d8.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d8.SetPositionAllocator (positionAllocd2d8);
  mobilityd2d8.Install (d2d[3].d2dNodes.Get (1));
  
  MobilityHelper mobilityd2d9;
  mobilityd2d9.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d9.SetPositionAllocator (positionAllocd2d9);
  mobilityd2d9.Install (d2d[4].d2dNodes.Get (0));
  
  MobilityHelper mobilityd2d10;
  mobilityd2d10.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityd2d10.SetPositionAllocator (positionAllocd2d10);
  mobilityd2d10.Install (d2d[4].d2dNodes.Get (1));
  
  
  
  Vector enb; 
  enb = positionAllocEnb->GetNext();
  
  Vector cu_positions[10]; 
  cu_positions[0] = positionAllocCu1->GetNext(); 
  cu_positions[1] = positionAllocCu2->GetNext();
  cu_positions[2] = positionAllocCu3->GetNext();
  
  Vector d2d_positions[10]; 
  d2d_positions[0] = positionAllocd2d1->GetNext();
  d2d_positions[1] = positionAllocd2d2->GetNext();
  d2d_positions[2] = positionAllocd2d3->GetNext();
  d2d_positions[3] = positionAllocd2d4->GetNext();
  d2d_positions[4] = positionAllocd2d5->GetNext();
  d2d_positions[5] = positionAllocd2d6->GetNext();
  d2d_positions[6] = positionAllocd2d7->GetNext();
  d2d_positions[7] = positionAllocd2d8->GetNext();
  d2d_positions[8] = positionAllocd2d9->GetNext();
  d2d_positions[9] = positionAllocd2d10->GetNext();
  
  for(int i=0 ; i<5 ; i++) d2d[i].store(d2d_positions[2*i], d2d_positions[2*i+1], i);
  for(int i=0 ; i<3 ; i++) cu[i].store(cu_positions[i]);
  
  
  
  //Install LTE devices to the nodes and fix the random number stream
  int64_t randomStream = 1;
  NetDeviceContainer enbDevs = lteHelper->InstallEnbDevice (enbNode);
  randomStream += lteHelper->AssignStreams (enbDevs, randomStream);
  NetDeviceContainer cuDevs1 = lteHelper->InstallUeDevice (cu[0].Node);
  randomStream += lteHelper->AssignStreams (cuDevs1, randomStream);
  NetDeviceContainer cuDevs2 = lteHelper->InstallUeDevice (cu[1].Node);
  randomStream += lteHelper->AssignStreams (cuDevs2, randomStream);
  NetDeviceContainer cuDevs3 = lteHelper->InstallUeDevice (cu[2].Node);
  randomStream += lteHelper->AssignStreams (cuDevs3, randomStream);
  NetDeviceContainer ueDevs1 = lteHelper->InstallUeDevice (d2d[0].d2dNodes);
  randomStream += lteHelper->AssignStreams (ueDevs1, randomStream);
  NetDeviceContainer ueDevs2 = lteHelper->InstallUeDevice (d2d[1].d2dNodes);
  randomStream += lteHelper->AssignStreams (ueDevs2, randomStream);
  NetDeviceContainer ueDevs3 = lteHelper->InstallUeDevice (d2d[2].d2dNodes);
  randomStream += lteHelper->AssignStreams (ueDevs3, randomStream);
  NetDeviceContainer ueDevs4 = lteHelper->InstallUeDevice (d2d[3].d2dNodes);
  randomStream += lteHelper->AssignStreams (ueDevs4, randomStream);
  NetDeviceContainer ueDevs5 = lteHelper->InstallUeDevice (d2d[4].d2dNodes);
  randomStream += lteHelper->AssignStreams (ueDevs5, randomStream);
  

  //Configure Sidelink Pool
  Ptr<LteSlEnbRrc> enbSidelinkConfiguration = CreateObject<LteSlEnbRrc> ();
  enbSidelinkConfiguration->SetSlEnabled (true);

  //Preconfigure pool for the group
  LteRrcSap::SlCommTxResourcesSetup pool;

  pool.setup = LteRrcSap::SlCommTxResourcesSetup::SCHEDULED;
  //BSR timers
  pool.scheduled.macMainConfig.periodicBsrTimer.period = LteRrcSap::PeriodicBsrTimer::sf16;
  pool.scheduled.macMainConfig.retxBsrTimer.period = LteRrcSap::RetxBsrTimer::sf640;
  //MCS
  pool.scheduled.haveMcs = true;
  pool.scheduled.mcs = 16;
  //resource pool
  LteSlResourcePoolFactory pfactory;
  pfactory.SetHaveUeSelectedResourceConfig (false); //since we want eNB to schedule

  //Control
  pfactory.SetControlPeriod ("sf40");
  pfactory.SetControlBitmap (0x00000000FF); //8 subframes for PSCCH
  pfactory.SetControlOffset (0);
  pfactory.SetControlPrbNum (22);
  pfactory.SetControlPrbStart (0);
  pfactory.SetControlPrbEnd (49);

  //Data: The ns3::RrSlFfMacScheduler is responsible to handle the parameters


  pool.scheduled.commTxConfig = pfactory.CreatePool ();

  uint32_t groupL2Address = 255;

  enbSidelinkConfiguration->AddPreconfiguredDedicatedPool (groupL2Address, pool);
  lteHelper->InstallSidelinkConfiguration (enbDevs, enbSidelinkConfiguration);

  //pre-configuration for the UEs
  Ptr<LteSlUeRrc> ueSidelinkConfiguration = CreateObject<LteSlUeRrc> ();
  ueSidelinkConfiguration->SetSlEnabled (true);
  LteRrcSap::SlPreconfiguration preconfiguration;
  ueSidelinkConfiguration->SetSlPreconfiguration (preconfiguration);
  lteHelper->InstallSidelinkConfiguration (ueDevs1, ueSidelinkConfiguration);
  lteHelper->InstallSidelinkConfiguration (ueDevs2, ueSidelinkConfiguration);
  lteHelper->InstallSidelinkConfiguration (ueDevs3, ueSidelinkConfiguration);
  lteHelper->InstallSidelinkConfiguration (ueDevs4, ueSidelinkConfiguration);
  lteHelper->InstallSidelinkConfiguration (ueDevs5, ueSidelinkConfiguration);

  InternetStackHelper internet;
  internet.Install (d2d[0].d2dNodes);
  internet.Install (d2d[1].d2dNodes);
  internet.Install (d2d[2].d2dNodes);
  internet.Install (d2d[3].d2dNodes);
  internet.Install (d2d[4].d2dNodes);
  internet.Install (cu[0].Node);
  internet.Install (cu[1].Node);
  internet.Install (cu[2].Node);
  Ipv4Address groupAddress4 ("225.0.0.0");     //use multicast address as destination
  Ipv6Address groupAddress6 ("ff0e::1");     //use multicast address as destination
  Address remoteAddress;
  Address localAddress;
  Ptr<LteSlTft> tft;
  if (!useIPv6)
    {
      //Install the IP stack on the UEs and assign IP address
      
      Ipv4InterfaceContainer ueIpIface1;
      ueIpIface1 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (cuDevs1));
      
      Ipv4InterfaceContainer ueIpIface2;
      ueIpIface2 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (cuDevs2));
      
      Ipv4InterfaceContainer ueIpIface3;
      ueIpIface3 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (cuDevs3));
      
      Ipv4InterfaceContainer ueIpIface4;
      ueIpIface4 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs1));
      
      Ipv4InterfaceContainer ueIpIface5;
      ueIpIface5 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs2));
      
      Ipv4InterfaceContainer ueIpIface6;
      ueIpIface6 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs3));
      
      Ipv4InterfaceContainer ueIpIface7;
      ueIpIface7 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs4));

      Ipv4InterfaceContainer ueIpIface8;
      ueIpIface8 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs5));
      

      // set the default gateway for the UE
      Ipv4StaticRoutingHelper ipv4RoutingHelper;
      for (uint32_t u = 0; u < d2d[0].d2dNodes.GetN (); ++u)
        {
          Ptr<Node> ueNode = d2d[0].d2dNodes.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }
        
      for (uint32_t u = 0; u < d2d[1].d2dNodes.GetN (); ++u)
        {
          Ptr<Node> ueNode = d2d[1].d2dNodes.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }
        
      for (uint32_t u = 0; u < d2d[2].d2dNodes.GetN (); ++u)
        {
          Ptr<Node> ueNode = d2d[2].d2dNodes.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }
        
      for (uint32_t u = 0; u < d2d[3].d2dNodes.GetN (); ++u)
        {
          Ptr<Node> ueNode = d2d[3].d2dNodes.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        } 
        
      for (uint32_t u = 0; u < d2d[4].d2dNodes.GetN (); ++u)
        {
          Ptr<Node> ueNode = d2d[4].d2dNodes.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }   
        
      for (uint32_t u = 0; u < cu[0].Node.GetN (); ++u)
        {
          Ptr<Node> ueNode = cu[0].Node.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }
        
      for (uint32_t u = 0; u < cu[1].Node.GetN (); ++u)
        {
          Ptr<Node> ueNode = cu[1].Node.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        }
        
       for (uint32_t u = 0; u < cu[2].Node.GetN (); ++u)
        {
          Ptr<Node> ueNode = cu[2].Node.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
        } 
        
        
      remoteAddress = InetSocketAddress (groupAddress4, 8000);
      localAddress = InetSocketAddress (Ipv4Address::GetAny (), 8000);
      tft = Create<LteSlTft> (LteSlTft::BIDIRECTIONAL, groupAddress4, groupL2Address);
    }
  else
    {
      Ipv6InterfaceContainer ueIpIface;
      ueIpIface = epcHelper->AssignUeIpv6Address (NetDeviceContainer (ueDevs1));

      // set the default gateway for the UE
      Ipv6StaticRoutingHelper Ipv6RoutingHelper;
      for (uint32_t u = 0; u < d2d[1].d2dNodes.GetN (); ++u)
        {
          Ptr<Node> ueNode = d2d[1].d2dNodes.Get (u);
          // Set the default gateway for the UE
          Ptr<Ipv6StaticRouting> ueStaticRouting = Ipv6RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv6> ());
          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress6 (), 1);
        }
      remoteAddress = Inet6SocketAddress (groupAddress6, 8000);
      localAddress = Inet6SocketAddress (Ipv6Address::GetAny (), 8000);
      tft = Create<LteSlTft> (LteSlTft::BIDIRECTIONAL, groupAddress6, groupL2Address);
    }

  //Attach each UE to the best available eNB
  lteHelper->Attach (ueDevs1, enbDevs.Get (0));
  lteHelper->Attach (ueDevs2, enbDevs.Get (0));
  lteHelper->Attach (ueDevs3, enbDevs.Get (0));
  lteHelper->Attach (ueDevs4, enbDevs.Get (0));
  lteHelper->Attach (ueDevs5, enbDevs.Get (0));
  lteHelper->Attach (cuDevs1, enbDevs.Get (0));
  lteHelper->Attach (cuDevs2, enbDevs.Get (0));
  lteHelper->Attach (cuDevs3, enbDevs.Get (0));
  
  for(int i=0; i<3; i++)
  {
    Ipv4Address localAddrs =  cu[i].Node.Get(0)->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
    cu[i].address = localAddrs;
  }
  
  for(int i=0; i<5; i++)
  {
    Ipv4Address localAddrs1 =  d2d[i].d2dNodes.Get(0)->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
    d2d[i].address1 = localAddrs1;
    Ipv4Address localAddrs2 =  d2d[i].d2dNodes.Get(1)->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
    d2d[i].address2 = localAddrs2;
  }
  
  
  int k=0, j=0;
  std::vector<d2d_user> public_safety;
  std::vector<d2d_user> social_media;
  
  
  // Creating two groups (i) social media (ii) public safety
  for(int i=0 ; i<5 ; i++) 
  {
   if (d2d[i].application_type==2)
   {
    public_safety.push_back(d2d[i]);
    j++;
   }
   else
   {
    social_media.push_back(d2d[i]);
    k++;
   }
  }
  
  std::vector<point> public_safety_centers;
  std::vector<point> social_media_centers;
  d2d_user *ptr;
  ptr = d2d;
  
  // k-means function call
  public_safety_centers = k_means(public_safety, 2, ptr);
  for(int i=0; i<j; i++) public_safety_centers[i].application = 2;
  social_media_centers = k_means(social_media, 1, ptr);
  for(int i=0; i<k; i++) social_media_centers[i].application = 1;
  
  std::vector<point> centers;
  for(int i=0; i<k; i++) public_safety_centers.push_back(social_media_centers[i]);
  centers = public_safety_centers;
  
  
  // assigning the nearest CU for a shard  
   for (int i = 0; i < 3; i++) {
        int flag=1; int temp;
	double* dists = new double[k];
	for (int p = 0; p < 3; p++) {
		double a = std::abs(centers[i].y - cu[p].position.y);	
		double b = std::abs(centers[i].x - cu[p].position.x);	
		dists[p] = std::sqrt(std::pow(a, 2) + std::pow(b, 2));	
	}
	while(flag)
	{
	 temp = std::min_element(dists, dists + 3) - dists;
	 if(cu[temp].occupied) dists[temp] = 1000000;
	 else 
	 {
	  cu[temp].application_type = centers[i].application;
	  cu[temp].occupied = 1;
	  cu[temp].cluster_id = centers[i].cluster;
	  flag=0;
	 } 
	}
	delete[] dists;
    }
    
  print(d2d,cu);   
  

  ///*** Configure applications ***///

  //Set Application in the UEs
  OnOffHelper sidelinkClient ("ns3::UdpSocketFactory", remoteAddress);
  sidelinkClient.SetConstantRate (DataRate ("16kb/s"), 200);
  
  ApplicationContainer clientApps_shard1;
  ApplicationContainer serverApps_shard1;
  int t=0; 
  
  for(int i=0; i<5; i++)
  {
     if(d2d[i].application_type == 2 && d2d[i].cluster_id == 0)
     { 
     
      ApplicationContainer clientApps1 = sidelinkClient.Install (d2d[i].d2dNodes.Get (0));
      clientApps1.Start (Seconds (t + 1.9));
      clientApps1.Stop (Seconds (11.0));
      clientApps_shard1.Add(clientApps1);
      shard1[t] = i;
      
      ApplicationContainer serverApps1;
      PacketSinkHelper sidelinkSink1 ("ns3::UdpSocketFactory", localAddress);
      serverApps1 = sidelinkSink1.Install (d2d[i].d2dNodes.Get (1));
      serverApps1.Start (Seconds (t+0.9));
      serverApps_shard1.Add(serverApps1);
      
      t++;
      size1++;
     }
  }
  

  ApplicationContainer clientApps_shard2;
  ApplicationContainer serverApps_shard2;
  t=0;
  
  for(int i=0; i<5; i++)
  {
     if(d2d[i].application_type == 2 && d2d[i].cluster_id == 1)
     { 
      ApplicationContainer clientApps2 = sidelinkClient.Install (d2d[i].d2dNodes.Get (0));
      clientApps2.Start (Seconds (t + 1.9));
      clientApps2.Stop (Seconds (11.0));
      clientApps_shard2.Add(clientApps2);
      shard2[t] = i;
      
      ApplicationContainer serverApps2;
      PacketSinkHelper sidelinkSink2 ("ns3::UdpSocketFactory", localAddress);
      serverApps2 = sidelinkSink2.Install (d2d[i].d2dNodes.Get (1));
      serverApps2.Start (Seconds (t+0.9));
      serverApps_shard2.Add(serverApps2);
      
      
      t++;
      size2++;
     }
  }
  
  ApplicationContainer clientApps_shard3;
  ApplicationContainer serverApps_shard3;
  t=0;
  
  for(int i=0; i<5; i++)
  {
     if(d2d[i].application_type == 1 && d2d[i].cluster_id == 0)
     { 
      ApplicationContainer clientApps3 = sidelinkClient.Install (d2d[i].d2dNodes.Get (0));
      clientApps3.Start (Seconds(t+1.9));
      clientApps3.Stop (Seconds (11.0));
      clientApps_shard3.Add(clientApps3);
      shard3[t] = i;
      
      ApplicationContainer serverApps3;
      PacketSinkHelper sidelinkSink3 ("ns3::UdpSocketFactory", localAddress);
      serverApps3 = sidelinkSink3.Install (d2d[i].d2dNodes.Get (1));
      serverApps3.Start (Seconds (t+0.9));
      serverApps_shard3.Add(serverApps3);
      
      
      t++;
      size3++;
     }
  }
  
  // Side-link activation
  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, ueDevs1, tft);
  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, ueDevs2, tft);
  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, ueDevs3, tft);
  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, ueDevs4, tft);
  proseHelper->ActivateSidelinkBearer (slBearersActivationTime, ueDevs5, tft);
  
  ///*** End of application configuration ***///

  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("UePacketTrace.tr");

  //Trace file table header
  *stream->GetStream () << "time(sec)\ttx/rx\tNodeID\tIMSI\tPktSize(bytes)\tIP[src]\tIP[dst]" << std::endl;

  std::ostringstream oss;

  if (!useIPv6)
    {
      // Set Tx traces
      
      for (uint16_t ac = 0; ac < clientApps_shard1.GetN (); ac++)
        {
          Ipv4Address localAddrs =  clientApps_shard1.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
          d2d[shard1[ac]].address1 = localAddrs;
          oss << "tx\t" << d2d[shard1[ac]].d2dNodes.Get (0)->GetId () << "\t" << d2d[shard1[ac]].d2dNodes.Get (0)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
          clientApps_shard1.Get (ac)->TraceConnect ("TxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
          oss.str ("");
        }  
      
      for (uint16_t ac = 0; ac < clientApps_shard2.GetN (); ac++)
        {
          Ipv4Address localAddrs =  clientApps_shard2.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
          d2d[shard2[ac]].address1 = localAddrs;
          oss << "tx\t" << d2d[shard2[ac]].d2dNodes.Get (0)->GetId () << "\t" << d2d[shard2[ac]].d2dNodes.Get (0)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
          clientApps_shard2.Get (ac)->TraceConnect ("TxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
          oss.str ("");
        }
         
      
      for (uint16_t ac = 0; ac < clientApps_shard3.GetN (); ac++)
        {
          Ipv4Address localAddrs =  clientApps_shard3.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
          d2d[shard3[ac]].address1 = localAddrs;
          oss << "tx\t" << d2d[shard3[ac]].d2dNodes.Get (0)->GetId () << "\t" << d2d[shard3[ac]].d2dNodes.Get (0)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
          clientApps_shard3.Get (ac)->TraceConnect ("TxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
          oss.str ("");
        } 
        

      // Set Rx traces
      
      for (uint16_t ac = 0; ac < serverApps_shard1.GetN (); ac++)
        {
          Ipv4Address localAddrs =  serverApps_shard1.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
          d2d[shard1[ac]].address2 = localAddrs;
          oss << "rx\t" << d2d[shard1[ac]].d2dNodes.Get (1)->GetId () << "\t" << d2d[shard1[ac]].d2dNodes.Get (1)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
          serverApps_shard1.Get (ac)->TraceConnect ("RxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
          oss.str ("");
        }
      
      for (uint16_t ac = 0; ac < serverApps_shard2.GetN (); ac++)
        {
          Ipv4Address localAddrs =  serverApps_shard2.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
          d2d[shard2[ac]].address2 = localAddrs;
          oss << "rx\t" << d2d[shard2[ac]].d2dNodes.Get (1)->GetId () << "\t" << d2d[shard2[ac]].d2dNodes.Get (1)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
          serverApps_shard2.Get (ac)->TraceConnect ("RxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
          oss.str ("");
        }
      
      for (uint16_t ac = 0; ac < serverApps_shard3.GetN (); ac++)
        {
          Ipv4Address localAddrs =  serverApps_shard3.Get (ac)->GetNode ()->GetObject<Ipv4L3Protocol> ()->GetAddress (1,0).GetLocal ();
          d2d[shard3[ac]].address2 = localAddrs;
          oss << "rx\t" << d2d[shard3[ac]].d2dNodes.Get (1)->GetId () << "\t" << d2d[shard3[ac]].d2dNodes.Get (1)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
          serverApps_shard3.Get (ac)->TraceConnect ("RxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
          oss.str ("");
        }
      
    }
  else
    {
      // Set Tx traces
      for (uint16_t ac = 0; ac < clientApps_shard1.GetN (); ac++)
        {
          clientApps_shard1.Get (ac)->GetNode ()->GetObject<Ipv6L3Protocol> ()->AddMulticastAddress (groupAddress6);
          Ipv6Address localAddrs =  clientApps_shard1.Get (ac)->GetNode ()->GetObject<Ipv6L3Protocol> ()->GetAddress (1,1).GetAddress ();
          std::cout << "Tx address: " << localAddrs << std::endl;
          oss << "tx\t" << d2d[1].d2dNodes.Get (0)->GetId () << "\t" << d2d[1].d2dNodes.Get (0)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
          clientApps_shard1.Get (ac)->TraceConnect ("TxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
          oss.str ("");
        }

      // Set Rx traces
      for (uint16_t ac = 0; ac < serverApps_shard1.GetN (); ac++)
        {
          serverApps_shard1.Get (ac)->GetNode ()->GetObject<Ipv6L3Protocol> ()->AddMulticastAddress (groupAddress6);
          Ipv6Address localAddrs =  serverApps_shard1.Get (ac)->GetNode ()->GetObject<Ipv6L3Protocol> ()->GetAddress (1,1).GetAddress ();
          std::cout << "Rx address: " << localAddrs << std::endl;
          oss << "rx\t" << d2d[1].d2dNodes.Get (1)->GetId () << "\t" << d2d[1].d2dNodes.Get (1)->GetDevice (0)->GetObject<LteUeNetDevice> ()->GetImsi ();
          serverApps_shard1.Get (ac)->TraceConnect ("RxWithAddresses", oss.str (), MakeBoundCallback (&UePacketTrace, stream, localAddrs));
          oss.str ("");
        }
    }

    
  NS_LOG_INFO ("Enabling Sidelink traces...");
  lteHelper->EnableSidelinkTraces ();

  NS_LOG_INFO ("Starting simulation...");

  Simulator::Stop (simTime);
  
  //std::cout<<"\n";
  //std::cout<<"    SINR values of Shard 1  \n";
  //std::cout<<"\n";
  
  //Config::Connect ("/NodeList/*/DeviceList/0/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/ReportCurrentCellRsrpSinr", MakeCallback (&PhySnirTrace1));

  std::cout<<"\n";
  std::cout<<"\n";
  std::cout<<"    SINR values of Shard 2  \n";
  std::cout<<"\n";
  
Config::Connect ("/NodeList/*/DeviceList/0/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/ReportCurrentCellRsrpSinr", MakeCallback (&PhySnirTrace2));


  //std::cout<<"\n";
  //std::cout<<"\n";

  //std::cout<<"    SINR values of Shard 3  \n";
  //std::cout<<"\n";
  
//Config::Connect ("/NodeList/*/DeviceList/0/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/ReportCurrentCellRsrpSinr", MakeCallback (&PhySnirTrace3));
  

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;

}
