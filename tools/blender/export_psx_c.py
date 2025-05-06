bl_info = {
    "name": "Export PSX C Model",
    "blender": (2, 79, 0),
    "category": "Export",
}

import bpy
import bmesh
import random
from mathutils import Vector

def write_psx_c(filepath, obj):
    # Получаем доступ к данным объекта и применяем модификаторы
    mesh = obj.to_mesh(scene=bpy.context.scene, apply_modifiers=True, settings='RENDER')
    bm = bmesh.new()
    bm.from_mesh(mesh)
    bm.verts.ensure_lookup_table()
    bm.faces.ensure_lookup_table()

    with open(filepath, 'w') as f:
        f.write("#include <SYS/TYPES.H>\n\n")

        # Вершины
        f.write("SVECTOR model_mesh[] = {\n")
        for v in bm.verts:
            co = v.co
            f.write("    {%d, %d, %d},\n" % (int(co.x * 64), int(co.y * 64), int(co.z * 64)))
        f.write("};\n\n")

        # Нормали
        f.write("SVECTOR model_normals[] = {\n")
        for v in bm.verts:
            n = v.normal.normalized()
            f.write("    {%d, %d, %d},\n" % (int(co.x * 64), int(co.y * 64), int(co.z * 64)))
        f.write("};\n\n")

        # Индексы и цвета
        indices = []
        colors = []
        for face in bm.faces:
            # Генерация случайного цвета для каждой грани
            r = random.randint(0, 255)
            g = random.randint(0, 255)
            b = random.randint(0, 255)

            if len(face.verts) == 3:
                idx = [v.index for v in face.verts]
                indices.append(idx)
                colors.append((r, g, b))
            elif len(face.verts) == 4:
                idx = [v.index for v in face.verts]
                # Разбиение квадрата на два треугольника
                indices.append([idx[0], idx[1], idx[2]])
                colors.append((r, g, b))
                indices.append([idx[0], idx[2], idx[3]])
                colors.append((r, g, b))

        # Запись индексов
        f.write("int model_indices[] = {\n")
        for idx in indices:
            f.write("    %d, %d, %d,\n" % (idx[0], idx[1], idx[2]))
        f.write("};\n\n")

        # Запись цветов
        f.write("CVECTOR model_colors[] = {\n")
        for color in colors:
            f.write("    %d, %d, %d, 0,\n" % (color[0], color[1], color[2]))
        f.write("};\n\n")

        # TMESH структура
        f.write("TMESH model = {\n")
        f.write("    model_mesh,\n")
        f.write("    model_normals,\n")
        f.write("    0,\n")
        f.write("    model_colors,\n")
        f.write("    %d\n" % len(bm.faces))
        f.write("};\n")

    bm.free()

class ExportPSXC(bpy.types.Operator):
    bl_idname = "export_scene.psx_c"
    bl_label = "Export PSX C Model"

    filepath = bpy.props.StringProperty(subtype="FILE_PATH")

    def execute(self, context):
        obj = context.active_object
        if obj.type != 'MESH':
            self.report({'ERROR'}, "Active object is not a mesh.")
            return {'CANCELLED'}
        write_psx_c(self.filepath, obj)
        self.report({'INFO'}, "Model exported successfully.")
        return {'FINISHED'}

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}

def menu_func_export(self, context):
    self.layout.operator(ExportPSXC.bl_idname, text="PSX C Model (.c)")

def register():
    bpy.utils.register_class(ExportPSXC)
    bpy.types.INFO_MT_file_export.append(menu_func_export)

def unregister():
    bpy.utils.unregister_class(ExportPSXC)
    bpy.types.INFO_MT_file_export.remove(menu_func_export)

if __name__ == "__main__":
    register()
