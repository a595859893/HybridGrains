using System.Collections;
using System.Collections.Generic;
using UnityEngine;


public static class Constant{
    public const float BSPLINE_EPSILON = 1e-10F;
    public const int REGION = 100; //网格化范围
    public const float KAPPA = 0.1f; // 物理系数
    public const float MU = 0.1f; // 物理系数
    public static Vector3Int[] ROUND = new Vector3Int[] { new Vector3Int(0, 0, 0), new Vector3Int(0, 0, 1), new Vector3Int(0, 0, -1), new Vector3Int(0, 1, 0), new Vector3Int(0, -1, 0), new Vector3Int(1, 0, 0), new Vector3Int(-1, 0, 0), };
}