//#include "GGX.hpp"
//#include "Raytracer.hpp"
//
//using namespace std;
//using namespace glm;
//
//
//
//
//
//
//
//dvec3 G(dvec3 i, dvec3 o, dvec3 m) {
//	/*The shadowing-masking function depends on the details
//of the microsurface, and exact expressions are rarely avail-
//able. More typically, approximations are derived using vari-
//ous statistical models and simplifying assumptions. See Sec-
//tions 5 and Appendix A for more discussion.*/
//}
//
//
//dvec3 fancyP(dvec3 i, dvec3 m) {
//
//}
//
//dvec3 h(dvec3 i, dvec3 o) {
//
//}
//
//dvec3 hr(dvec3 i, dvec3 o) {
//
//	//hrarrow = sign(i · n) (i + o) 
//	//hr = hrarrow/() || hrarrow || )
//
//}
//
//double absJacobianDeterm(dvec3 h, dvec3 o) {
//	/*the absolute value of the determinant of the
//Jacobian matrix for the transform between h and o (using
//solid angle measures). For brevity, the latter is often simply
//called the Jacobian.*/
//}
//
//double F(dvec3 i, dvec3 m) {
//	
//}
//
//
//double diracDelta() {
//
//}
//
//double fr(dvec3 i, dvec3 o, dvec3 m) {
//	
//}
//
//dvec3 fs(dvec3 i, dvec3 o, dvec3 n) {
//	//section 3.3
//}
//
//
//
//
//dvec3 ggx(
//	const Ray& incomingRay,
//	const Ray& outgoingRay,
//	const HitResult& minRayResult,
//	dvec3 downstreamRadiance,
//	Material* mat, int numchar) {
//
//	dvec3 i = outgoingRay.inverseDirection;
//	dvec3 o = incomingRay.inverseDirection;
//	dvec3 n = minRayResult.normal;
//	dvec3 m = minRayResult.normal;//TODO: this is not correct, should be microsurface normal, which is different, maybe a random normal in the hemisphere based on the roughness? idk
//	
//
//
//
//
//	switch (numchar) {
//	case 0://transmit
//		break;
//	case 1://perfect reflect
//		break;
//	case 2://scatter
//		break;
//	default:
//		printf("illegal case passed to ggx\n");
//		exit(-1);
//		break;
//	}
//
//
//}
//
//
//
