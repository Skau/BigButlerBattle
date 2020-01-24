import bpy;
import os;

directory = os.path.dirname(bpy.data.filepath);
blendName = os.path.splitext(bpy.path.basename(bpy.context.blend_data.filepath));
filePath = directory + "\DT_" + blendName[0] + ".csv";
file = open(filePath, "w");
counter = 0;

file.write('---,Position,InTangent,OutTangent\n');
for ob in bpy.context.selected_objects:
	if ob.type == 'CURVE':
		for spline in ob.data.splines:
			if len(spline.bezier_points) > 0:
				for bezier_point in spline.bezier_points.values():
					file.write('%s,' % (ob.name + str(counter)));
					co = ob.matrix_world @ bezier_point.co;
					handle_in = ob.matrix_world @ bezier_point.handle_right;
					handle_out = ob.matrix_world @ bezier_point.handle_left;
					file.write('"(X=%.3f,Y=%.3f,Z=%.3f)",' % (co.x, co.y, co.z));
					file.write('"(X=%.3f,Y=%.3f,Z=%.3f)",' % (handle_in.x - co.x, handle_in.y - co.y, handle_in.z - co.z));
					file.write('"(X=%.3f,Y=%.3f,Z=%.3f)"\n' % (handle_out.x - co.x, handle_out.y - co.y, handle_out.z - co.z));
					counter += 1;

file.close();
