#include <yaml-cpp/yaml.h>
#include <DirectXMath.h>

namespace YAML {

    template<>
    struct convert<DirectX::XMFLOAT2> 
    {
        static Node encode(const DirectX::XMFLOAT2 &rhs) 
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node &node, DirectX::XMFLOAT2 &rhs) 
        {
            if (!node.IsSequence() || node.size() != 2)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            return true;
        }
    };

    template<>
    struct convert<DirectX::XMFLOAT3> 
    {
        static Node encode(const DirectX::XMFLOAT3 &rhs) 
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node &node, DirectX::XMFLOAT3 &rhs) 
        {
            if (!node.IsSequence() || node.size() != 3)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };
}