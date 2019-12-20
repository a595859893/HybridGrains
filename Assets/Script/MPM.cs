using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using static Unity.Mathematics.math;
using Unity.Mathematics;

public class GridNode
{
    public float m;
    public Vector3 p;
    public Vector3 f;
}

public class MaterialPoint
{
    public Transform transform;
    public float m;
    public Vector3 v;
    public float J;
    public float3x3 sigma;
    public float3x3 be;
}


public class Grid
{
    float cellSize;
    GridNode[] nodes;
    Vector3 pos;
    int range;
    public Grid(Vector3 pos, float cellSize, int range)
    {
        this.cellSize = cellSize;
        this.pos = pos;
        this.range = range;
        nodes = new GridNode[range * range * range];
    }
    public GridNode GetNodes(Vector3Int gridVec)
    {
        return nodes[gridVec.z * range * range + gridVec.y * range + gridVec.x];
    }

    public void ResizeMassAndMomentumToGrid(MaterialPoint[] points)
    {
        System.Array.Clear(nodes, 0, nodes.Length);
        foreach (MaterialPoint point in points)
        {
            Vector3 pointPos = point.transform.position;
            Vector3 gridPos = (pointPos - pos) / cellSize;
            Vector3Int gridOrgVec = Vector3Int.RoundToInt(gridPos);
            foreach (Vector3Int round in Constant.ROUND)
            {
                Vector3Int gridVec = gridOrgVec + round;
                GridNode node = GetNodes(gridVec);

                float w = Weight(point, gridVec);
                node.m += w * point.m;
                node.p += w * point.m * point.v;
            }
        }
    }

    public void ComputeForcesOnGrid(MaterialPoint[] points)
    {
        foreach (MaterialPoint point in points)
        {
            Vector3 pointPos = point.transform.position;
            Vector3 gridPos = (pointPos - pos) / cellSize;
            Vector3Int gridOrgVec = Vector3Int.RoundToInt(gridPos);
            foreach (Vector3Int round in Constant.ROUND)
            {
                Vector3Int gridVec = gridOrgVec + round;
                GridNode node = GetNodes(gridVec);

                Vector3 w = WeightGard(point, gridVec);
                // node.f += -point.v * point.J * point.sigma * w;

            }
        }
    }

    float Weight(MaterialPoint point, Vector3Int gridVec)
    {
        Vector3 pointPos = point.transform.position;
        Vector3 gridPos = (pointPos - pos) / cellSize;
        Vector3 vpos = gridPos - gridVec;

        float wx = bspline(vpos.x);
        float wy = bspline(vpos.y);
        float wz = bspline(vpos.z);
        return wx * wy * wz;
    }

    Vector3 WeightGard(MaterialPoint point, Vector3Int gridVec)
    {
        Vector3 pointPos = point.transform.position;
        Vector3 gridPos = (pointPos - pos) / cellSize;
        Vector3 vpos = gridPos - gridVec;

        float wx = bspline(vpos.x);
        float wy = bspline(vpos.y);
        float wz = bspline(vpos.z);

        float dx = bsplineSlope(vpos.x);
        float dy = bsplineSlope(vpos.y);
        float dz = bsplineSlope(vpos.z);

        Vector3 gard = new Vector3(dx * wy * wz, dy * wx * wz, dz * wx * wy);
        gard /= cellSize;
        return gard;
    }

    float bspline(float x)
    {
        float w;
        x = Mathf.Abs(x);
        if (x < 1)
        {
            w = x * x * (x / 2 - 1) + 2 / 3.0f;
        }
        else if (x < 2)
        {
            w = x * (x * (-x / 6 + 1 - 2)) + 4 / 3.0f;
        }
        else
        {
            return 0;
        }

        return (w < Constant.BSPLINE_EPSILON) ? 0 : w;
    }


    float bsplineSlope(float x)
    {
        float absX = Mathf.Abs(x);
        if (x < 1)
        {
            return 1.5f * x * absX - 2 * x;
        }
        else if (x < 2)
        {
            return -x * absX / 2 + 2 * x - 2 * x / absX;
        }

        return 0;
    }

}

public class MPM
{
    Grid grid;
    MaterialPoint[] points;
    public MPM()
    {
        grid = new Grid(Vector3.zero, 1, 50);
    }

    void MpmFirst()
    {
        grid.ResizeMassAndMomentumToGrid(points);
    }

    void ComputeStressAtPoints()
    {
        foreach (MaterialPoint point in points)
        {
            Matrix4x4 tau = Matrix4x4.identity;
            if (point.J <= 1)
            {
                // tau = Constant.KAPPA * 0.5f * point.J * point.J * Matrix4x4.identity + Constant.MU * Matrix4x4.Determinant(point.be);
                // TODO: 不会算张量……
            }
        }
    }
}

