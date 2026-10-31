#pragma once

#include <cstdint>
#include <obj_loader.hpp>

ObjMesh generateUvSphere(uint32_t segmentCount, uint32_t ringCount);
ObjMesh generateIcoSphere(uint32_t subdivisionLevel);
