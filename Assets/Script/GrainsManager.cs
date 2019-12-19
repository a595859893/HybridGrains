using System.Collections;
using System.Collections.Generic;
using UnityEngine;



public class GrainsManager : MonoBehaviour
{
    const int SIZE = 10; //粒子个数

    public GameObject prefabs;
    List<Transform> grains = new List<Transform>(SIZE * SIZE * SIZE);
    Dictionary<Vector3Int, MaterialPoint> nodes = new Dictionary<Vector3Int, MaterialPoint>();
    // Start is called before the first frame update
    void Start()
    {
        for (int i = 0; i < SIZE; i++)
        {
            for (int j = 0; j < SIZE; j++)
            {
                for (int k = 0; k < SIZE; k++)
                {
                    Vector3 pos = new Vector3(5.0f - i, 5.0f - j, 5.0f - k);
                    GameObject obj = GameObject.Instantiate(prefabs, pos, Quaternion.identity) as GameObject;
                    obj.transform.parent = transform;
                    grains.Add(obj.transform);
                }
            }
        }
    }

    // Update is called once per frame
    void Update()
    {

    }
}
