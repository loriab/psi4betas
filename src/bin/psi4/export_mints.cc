#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <libmints/mints.h>
#include <libmints/integralparameters.h>
#include <libmints/orbitalspace.h>
#include <libmints/view.h>
#include <lib3index/3index.h>
#include <libscf_solver/hf.h>
#include <libscf_solver/rhf.h>

#include <string>

using namespace boost;
using namespace boost::python;
using namespace psi;

boost::shared_ptr<Vector> py_nuclear_dipole(shared_ptr<Molecule> mol)
{
    //SharedMolecule mol = Process::environment.molecule();
    return DipoleInt::nuclear_contribution(mol, Vector3(0, 0, 0));
}

boost::shared_ptr<MatrixFactory> get_matrix_factory()
{
    // We need a valid molecule with a valid point group to create a matrix factory.
    boost::shared_ptr<Molecule> molecule = Process::environment.molecule();
    if (!molecule) {
        fprintf(outfile, "  Active molecule not set!");
        throw PSIEXCEPTION("Active molecule not set!");
    }
    if (!molecule->point_group()) {
        fprintf(outfile, "  Active molecule does not have point group set!");
        throw PSIEXCEPTION("Active molecule does not have point group set!");
    }

    // Read in the basis set
    boost::shared_ptr<BasisSetParser> parser(new Gaussian94BasisSetParser);
    boost::shared_ptr<BasisSet> basis = BasisSet::construct(parser, molecule, "BASIS");
    boost::shared_ptr<IntegralFactory> fact(new IntegralFactory(basis, basis, basis, basis));
    boost::shared_ptr<SOBasisSet> sobasis(new SOBasisSet(basis, fact));
    const Dimension& dim = sobasis->dimension();

    boost::shared_ptr<MatrixFactory> matfac(new MatrixFactory);
    matfac->init_with(dim, dim);

    return matfac;
}

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(CanonicalOrthog, Matrix::canonical_orthogonalization, 1, 2)

void export_mints()
{
    def("nuclear_dipole", py_nuclear_dipole, "docstring");

    // This is needed to wrap an STL vector into Boost.Python. Since the vector
    // is going to contain boost::shared_ptr's we MUST set the no_proxy flag to true
    // (as it is) to tell Boost.Python to not create a proxy class to handle
    // the vector's data type.
    class_<std::vector<SharedMatrix > >("matrix_vector", "docstring").
            def(vector_indexing_suite<std::vector<SharedMatrix >, true >());

    // Use typedefs to explicitly tell Boost.Python which function in the class
    // to use. In most cases, you should not be making Python specific versions
    // of functions.

    // For example in Vector there are 2 versions of set: a (double*) version and a
    // (int, int, double) version. We create a typedef function pointer to tell
    // Boost.Python we only want the (int, int, double) version.
    typedef void (Vector::*vector_setitem_1)(int, double);
    typedef void (Vector::*vector_setitem_2)(int, int, double);
    typedef double (Vector::*vector_getitem_1)(int);
    typedef double (Vector::*vector_getitem_2)(int, int);
    typedef void (Vector::*vector_setitem_n)(const boost::python::tuple&, double);
    typedef double (Vector::*vector_getitem_n)(const boost::python::tuple&);

    class_<Dimension>("Dimension", "docstring").
            def(init<int>()).
            def(init<int, const std::string&>()).
            def("init", &Dimension::init, "docstring").
            def("n", &Dimension::n, return_value_policy<copy_const_reference>(), "docstring").
            def("name", &Dimension::name, return_value_policy<copy_const_reference>(), "docstring").
            def("set_name", &Dimension::set_name, "docstring").
            def("__getitem__", &Dimension::get, return_value_policy<copy_const_reference>(), "docstring").
            def("__setitem__", &Dimension::set, "docstring");

    class_<Vector, boost::shared_ptr<Vector> >( "Vector", "docstring").
            def(init<int>()).
            def("get", vector_getitem_1(&Vector::get), "docstring").
            def("get", vector_getitem_2(&Vector::get), "docstring").
            def("set", vector_setitem_1(&Vector::set), "docstring").
            def("set", vector_setitem_2(&Vector::set), "docstring").
            def("print_out", &Vector::print_out, "docstring").
            def("scale", &Vector::scale, "docstring").
            def("dim", &Vector::dim, "docstring").
            def("__getitem__", vector_getitem_1(&Vector::pyget), "docstring").
            def("__setitem__", vector_setitem_1(&Vector::pyset), "docstring").
            def("__getitem__", vector_getitem_n(&Vector::pyget), "docstring").
            def("__setitem__", vector_setitem_n(&Vector::pyset), "docstring").
            def("nirrep", &Vector::nirrep, "docstring");

    typedef void  (IntVector::*int_vector_set)(int, int, int);
    class_<IntVector, boost::shared_ptr<IntVector> >( "IntVector", "docstring").
            def(init<int>()).
            def("get", &IntVector::get, "docstring").
            def("set", int_vector_set(&IntVector::set), "docstring").
            def("print_out", &IntVector::print_out, "docstring").
            def("dim", &IntVector::dim, "docstring").
            def("nirrep", &IntVector::nirrep, "docstring");

    enum_<diagonalize_order>("DiagonalizeOrder", "docstring")
            .value("Ascending", ascending)
            .value("Descending", descending)
            .export_values();

    typedef void   (Matrix::*matrix_multiply)(bool, bool, double, const SharedMatrix&, const SharedMatrix&, double);
    typedef void   (Matrix::*matrix_diagonalize)(SharedMatrix&, boost::shared_ptr<Vector>&, diagonalize_order);
    typedef void   (Matrix::*matrix_one)(const SharedMatrix&);
    typedef double (Matrix::*double_matrix_one)(const SharedMatrix&);
    typedef void   (Matrix::*matrix_two)(const SharedMatrix&, const SharedMatrix&);
    typedef void   (Matrix::*matrix_save)(const std::string&, bool, bool, bool);
    typedef void   (Matrix::*matrix_set4)(int, int, int, double);
    typedef void   (Matrix::*matrix_set3)(int, int, double);
    typedef double (Matrix::*matrix_get3)(const int&, const int&, const int&) const;
    typedef double (Matrix::*matrix_get2)(const int&, const int&) const;
    typedef void   (Matrix::*matrix_load)(const std::string&);
    typedef const Dimension& (Matrix::*matrix_ret_dimension)() const;

    class_<Matrix, SharedMatrix>("Matrix", "docstring").
            def(init<int, int>()).
            def(init<const std::string&, const Dimension&, const Dimension&>()).
            def(init<const std::string&>()).
            def("clone", &Matrix::clone, "docstring").
            def("set_name", &Matrix::set_name, "docstring").
            def("name", &Matrix::name, return_value_policy<copy_const_reference>(), "docstring").
            def("print_out", &Matrix::print_out, "docstring").
            def("rows", &Matrix::rowdim, "docstring").
            def("cols", &Matrix::coldim, "docstring").
            def("rowdim", matrix_ret_dimension(&Matrix::rowspi), return_value_policy<copy_const_reference>(), "docstring").
            def("coldim", matrix_ret_dimension(&Matrix::colspi), return_value_policy<copy_const_reference>(), "docstring").
            def("nirrep", &Matrix::nirrep, return_value_policy<copy_const_reference>(), "docstring").
            def("symmetry", &Matrix::symmetry, return_value_policy<copy_const_reference>(), "docstring").
            def("identity", &Matrix::identity, "docstring").
            def("copy_lower_to_upper", &Matrix::copy_lower_to_upper, "docstring").
            def("copy_upper_to_lower", &Matrix::copy_upper_to_lower, "docstring").
            def("zero_lower", &Matrix::zero_lower, "docstring").
            def("zero_upper", &Matrix::zero_upper, "docstring").
            def("zero", &Matrix::zero, "docstring").
            def("zero_diagonal", &Matrix::zero_diagonal, "docstring").
            def("trace", &Matrix::trace, "docstring").
            //            def("transpose", &Matrix::transpose).
            def("add", matrix_one(&Matrix::add), "docstring").
            def("subtract", matrix_one(&Matrix::subtract), "docstring").
            def("accumulate_product", matrix_two(&Matrix::accumulate_product), "docstring").
            def("scale", &Matrix::scale, "docstring").
            def("sum_of_squares", &Matrix::sum_of_squares, "docstring").
            def("add_and_orthogonalize_row", &Matrix::add_and_orthogonalize_row, "docstring").
            def("rms", &Matrix::rms, "docstring").
            def("scale_row", &Matrix::scale_row, "docstring").
            def("scale_column", &Matrix::scale_column, "docstring").
            def("transform", matrix_one(&Matrix::transform), "docstring").
            def("transform", matrix_two(&Matrix::transform), "docstring").
            def("transform", matrix_one(&Matrix::back_transform), "docstring").
            def("back_transform", matrix_two(&Matrix::back_transform), "docstring").
            def("vector_dot", double_matrix_one(&Matrix::vector_dot), "docstring").
            def("gemm", matrix_multiply(&Matrix::gemm), "docstring").
            def("diagonalize", matrix_diagonalize(&Matrix::diagonalize), "docstring").
            def("cholesky_factorize", &Matrix::cholesky_factorize, "docstring").
            def("partial_cholesky_factorize", &Matrix::partial_cholesky_factorize, "docstring").
            def("canonical_orthogonalization", &Matrix::canonical_orthogonalization, CanonicalOrthog()).
            def("invert", &Matrix::invert, "docstring").
            def("power", &Matrix::power, "docstring").
            def("get", matrix_get3(&Matrix::get), "docstring").
            def("get", matrix_get2(&Matrix::get), "docstring").
            def("set", matrix_set3(&Matrix::set), "docstring").
            def("set", matrix_set4(&Matrix::set), "docstring").
            def("set", &Matrix::set_by_python_list, "docstring").
            def("project_out", &Matrix::project_out, "docstring").
            def("__getitem__", &Matrix::pyget, "docstring").
            def("__setitem__", &Matrix::pyset, "docstring").
            def("save", matrix_save(&Matrix::save), "docstring").
            def("load", matrix_load(&Matrix::load), "docstring").
            def("load_mpqc", matrix_load(&Matrix::load_mpqc), "docstring").
            def("remove_symmetry", &Matrix::remove_symmetry, "docstring");

    class_<View, boost::noncopyable>("View", no_init).
            def(init<SharedMatrix, const Dimension&, const Dimension&>()).
            def(init<SharedMatrix, const Dimension&, const Dimension&, const Dimension&, const Dimension&>()).
            def("__call__", &View::operator(), "docstring");

    typedef SharedMatrix (MatrixFactory::*create_shared_matrix)();
    typedef SharedMatrix (MatrixFactory::*create_shared_matrix_name)(const std::string&);

    class_<MatrixFactory, boost::shared_ptr<MatrixFactory> >("MatrixFactory", "docstring").
            def("shared_object", &get_matrix_factory, "docstring").
            staticmethod("shared_object").
            def("create_matrix", create_shared_matrix(&MatrixFactory::create_shared_matrix), "docstring").
            def("create_matrix", create_shared_matrix_name(&MatrixFactory::create_shared_matrix), "docstring");

    class_<CdSalcList, boost::shared_ptr<CdSalcList>, boost::noncopyable>("CdSalcList", "docstring", no_init).
            def("print_out", &CdSalcList::print, "docstring").
            def("matrix", &CdSalcList::matrix, "docstring");

    typedef boost::shared_ptr<PetiteList> (MintsHelper::*petite_list_0)() const;
    typedef boost::shared_ptr<PetiteList> (MintsHelper::*petite_list_1)(bool) const;

    typedef SharedMatrix (MintsHelper::*erf)(double, SharedMatrix, SharedMatrix, SharedMatrix, SharedMatrix);
    typedef SharedMatrix (MintsHelper::*eri)(SharedMatrix, SharedMatrix, SharedMatrix, SharedMatrix);

    class_<MintsHelper, boost::shared_ptr<MintsHelper> >("MintsHelper", "docstring").
            def(init<boost::shared_ptr<BasisSet> >()).
            def("integrals", &MintsHelper::integrals, "docstring").
            def("integrals_erf", &MintsHelper::integrals_erf, "docstring").
            def("integrals_erfc", &MintsHelper::integrals_erfc, "docstring").
            def("one_electron_integrals", &MintsHelper::one_electron_integrals, "docstring").
            def("basisset", &MintsHelper::basisset, "docstring").
            def("sobasisset", &MintsHelper::sobasisset, "docstring").
            def("factory", &MintsHelper::factory, "docstring").
            def("ao_overlap", &MintsHelper::ao_overlap, "docstring").
            def("ao_kinetic", &MintsHelper::ao_kinetic, "docstring").
            def("ao_potential", &MintsHelper::ao_potential, "docstring").
            def("one_electron_integrals", &MintsHelper::one_electron_integrals, "docstring").
            def("so_overlap", &MintsHelper::so_overlap, "docstring").
            def("so_kinetic", &MintsHelper::so_kinetic, "docstring").
            def("so_potential", &MintsHelper::so_potential, "docstring").
            def("so_dipole", &MintsHelper::so_dipole, "docstring").
            def("so_quadrupole", &MintsHelper::so_quadrupole, "docstring").
            def("so_traceless_quadrupole", &MintsHelper::so_traceless_quadrupole, "docstring").
            def("ao_nabla", &MintsHelper::ao_nabla, "docstring").
            def("so_nabla", &MintsHelper::so_nabla, "docstring").
            def("so_angular_momentum", &MintsHelper::so_angular_momentum, "docstring").
            def("ao_angular_momentum", &MintsHelper::ao_angular_momentum, "docstring").
            def("ao_eri", &MintsHelper::ao_eri, "docstring").
            def("ao_erf_eri", &MintsHelper::ao_erf_eri, "docstring").
            def("ao_f12", &MintsHelper::ao_f12, "docstring").
            def("ao_f12_squared", &MintsHelper::ao_f12_squared, "docstring").
            def("ao_f12g12", &MintsHelper::ao_f12g12, "docstring").
            def("ao_f12_double_commutator", &MintsHelper::ao_f12_double_commutator, "docstring").
            def("mo_eri", eri(&MintsHelper::mo_eri), "docstring").
            def("mo_erf_eri", erf(&MintsHelper::mo_erf_eri), "docstring").
            def("mo_f12", &MintsHelper::mo_f12, "docstring").
            def("mo_f12_squared", &MintsHelper::mo_f12_squared, "docstring").
            def("mo_f12g12", &MintsHelper::mo_f12g12, "docstring").
            def("mo_f12_double_commutator", &MintsHelper::mo_f12_double_commutator, "docstring").
            def("cdsalcs", &MintsHelper::cdsalcs, "docstring").
            def("petite_list", petite_list_0(&MintsHelper::petite_list), "docstring").
            def("petite_list1", petite_list_1(&MintsHelper::petite_list), "docstring").
            def("play", &MintsHelper::play, "docstring");

    class_<FittingMetric, boost::shared_ptr<FittingMetric> >("FittingMetric", "docstring").
            def("get_algorithm", &FittingMetric::get_algorithm, "docstring").
            def("is_poisson", &FittingMetric::is_poisson, "docstring").
            def("is_inverted", &FittingMetric::is_inverted, "docstring").
            def("get_metric", &FittingMetric::get_metric, "docstring").
            def("get_pivots", &FittingMetric::get_pivots, "docstring").
            def("get_reverse_pivots", &FittingMetric::get_reverse_pivots, "docstring").
            def("form_fitting_metric", &FittingMetric::form_fitting_metric, "docstring").
            def("form_cholesky_inverse", &FittingMetric::form_cholesky_inverse, "docstring").
            def("form_QR_inverse", &FittingMetric::form_QR_inverse, "docstring").
            def("form_eig_inverse", &FittingMetric::form_eig_inverse, "docstring").
            def("form_full_inverse", &FittingMetric::form_full_inverse, "docstring");

    class_<PseudoTrial, boost::shared_ptr<PseudoTrial> >("PseudoTrial", "docstring").
            def("getI", &PseudoTrial::getI, "docstring").
            def("getIPS", &PseudoTrial::getIPS, "docstring").
            def("getQ", &PseudoTrial::getQ, "docstring").
            def("getR", &PseudoTrial::getR, "docstring").
            def("getA", &PseudoTrial::getA, "docstring");

    class_<Vector3>("Vector3", "Class for vectors of length three, often Cartesian coordinate vectors, and their common operations").
            def(init<double>()).
            def(init<double, double, double>()).
            def(init<const Vector3&>()).
            //      def(self = other<double>()).
            def(self += self).
            def(self -= self).
            def(self *= other<double>()).
            def(self + self).
            def(self - self).
            def(-self).
            def("dot", &Vector3::dot, "Returns dot product of arg1 and arg2").
            def("distance", &Vector3::distance, "Returns distance between two points represented by arg1 and arg2").
            def("normalize", &Vector3::normalize, "Returns vector of unit length and arg1 direction").
            def("norm", &Vector3::norm, "Returns Euclidean norm of arg1").
            def("cross", &Vector3::cross, "Returns cross product of arg1 and arg2").
            def("__str__", &Vector3::to_string, "Returns a string representation of arg1, suitable for printing.").
            def("__getitem__", &Vector3::get, "Returns the arg2-th element of arg1.");

    typedef void (SymmetryOperation::*intFunction)(int);
    typedef void (SymmetryOperation::*doubleFunction)(double);

    class_<SymmetryOperation>("SymmetryOperation", "docstring").
            def(init<const SymmetryOperation& >()).
            def("trace", &SymmetryOperation::trace, "docstring").
            def("zero", &SymmetryOperation::zero, "docstring").
            def("operate", &SymmetryOperation::operate, "docstring").
            def("transform", &SymmetryOperation::transform, "docstring").
            def("unit", &SymmetryOperation::unit, "docstring").
            def("E", &SymmetryOperation::E, "docstring").
            def("i", &SymmetryOperation::i, "docstring").
            def("sigma_xy", &SymmetryOperation::sigma_xy, "docstring").
            def("sigma_yz", &SymmetryOperation::sigma_yz, "docstring").
            def("sigma_xz", &SymmetryOperation::sigma_xz, "docstring").
            //        def("sigma_yz", &SymmetryOperation::sigma_yz).
            def("rotate_n", intFunction(&SymmetryOperation::rotation), "docstring").
            def("rotate_theta", doubleFunction(&SymmetryOperation::rotation), "docstring").
            def("c2_x", &SymmetryOperation::c2_x, "docstring").
            def("c2_y", &SymmetryOperation::c2_y, "docstring").
            def("transpose", &SymmetryOperation::transpose, "docstring");

    class_<OrbitalSpace>("OrbitalSpace", "docstring", no_init).
            def(init<const std::string&, const std::string&, const SharedMatrix&, const SharedVector&, const boost::shared_ptr<BasisSet>&, const boost::shared_ptr<IntegralFactory>& >()).
            def(init<const std::string&, const std::string&, const SharedMatrix&, const boost::shared_ptr<BasisSet>&, const boost::shared_ptr<IntegralFactory>& >()).
            def(init<const std::string&, const std::string&, const boost::shared_ptr<Wavefunction>& >()).
            def("nirrep", &OrbitalSpace::nirrep, "docstring").
            def("id", &OrbitalSpace::id, return_value_policy<copy_const_reference>(), "docstring").
            def("name", &OrbitalSpace::name, return_value_policy<copy_const_reference>(), "docstring").
            def("C", &OrbitalSpace::C, return_value_policy<copy_const_reference>(), "docstring").
            def("evals", &OrbitalSpace::evals, return_value_policy<copy_const_reference>(), "docstring").
            def("basisset", &OrbitalSpace::basisset, return_value_policy<copy_const_reference>(), "docstring").
            def("integral", &OrbitalSpace::integral, return_value_policy<copy_const_reference>(), "docstring").
            def("dim", &OrbitalSpace::dim, return_value_policy<copy_const_reference>(), "docstring").
            def("print_out", &OrbitalSpace::print, "docstring").
            def("build_cabs_space", &OrbitalSpace::build_cabs_space, "docstring").
            staticmethod("build_cabs_space").
            def("build_ri_space", &OrbitalSpace::build_ri_space, "docstring").
            staticmethod("build_ri_space");

    class_<PointGroup, boost::shared_ptr<PointGroup> >("PointGroup", "docstring").
            def(init<const std::string&>()).
            def("symbol", &PointGroup::symbol, "docstring");
            //def("origin", &PointGroup::origin).
//            def("set_symbol", &PointGroup::set_symbol);

    typedef void (Molecule::*matrix_set_geometry)(const Matrix &);

    class_<Molecule, boost::shared_ptr<Molecule> >("Molecule", "Class to store the elements, coordinates, fragmentation pattern, basis sets, charge, multiplicity, etc. of a molecule.").
            def("set_geometry", matrix_set_geometry(&Molecule::set_geometry), "Sets the geometry, given a (Natom X 3) matrix arg2 of coordinates (in Bohr)").
            def("set_name", &Molecule::set_name, "Sets molecule name").
            def("name", &Molecule::name, "Gets molecule name").
            def("reinterpret_coordentry", &Molecule::set_reinterpret_coordentry, "Do reinterpret coordinate entries during update_geometry().").
            def("fix_orientation", &Molecule::set_orientation_fixed, "Fix the orientation at its current frame").
            //def("fix_com", &Molecule::set_com_fixed).
            def("init_with_checkpoint", &Molecule::init_with_chkpt, "Populate arg1 member data with information from checkpoint file arg2").
            def("save_to_checkpoint", &Molecule::save_to_chkpt, "Saves molecule information to checkpoint file arg2 with prefix arg3").
            def("init_with_io", &Molecule::init_with_psio, "Creates a new checkpoint file with information from arg2").
            def("add_atom", &Molecule::add_atom, "Adds to Molecule arg1 an atom with atomic number arg2, Cartesian coordinates in Bohr (arg3, arg4, arg5), atomic symbol arg6, mass arg7, charge arg8 (optional), and lineno arg9 (optional)").
            def("natom", &Molecule::natom, "Number of real atoms").
            def("multiplicity", &Molecule::multiplicity, "Gets the multiplicity (defined as 2Ms + 1)").
            def("nfragments", &Molecule::nfragments, "Gets the number of fragments in the molecule").
            def("print_in_input_format", &Molecule::print_in_input_format, "Prints the molecule as Cartesian or ZMatrix entries, just as inputted.").
            def("save_xyz", &Molecule::save_xyz, "Saves an XYZ file to arg2").
            def("save_string_xyz", &Molecule::save_string_xyz, "Saves the string of an XYZ file to arg2").
            def("Z", &Molecule::Z, return_value_policy<copy_const_reference>(), "Nuclear charge of atom").
            def("x", &Molecule::x, "x position of atom").
            def("y", &Molecule::y, "y position of atom").
            def("z", &Molecule::z, "z position of atom").
            //def("xyz", &Molecule::xyz).
            def("center_of_mass", &Molecule::center_of_mass, "Computes center of mass of molecule (does not translate molecule)").
            def("translate", &Molecule::translate, "Translates molecule by arg2").
            def("move_to_com", &Molecule::move_to_com, "Moves molecule to center of mass").
            def("mass", &Molecule::mass, "Gets mass of atom arg2").
            def("symbol", &Molecule::symbol, "Gets the cleaned up label of atom arg2 (C2 => C, H4 = H)").
            def("label", &Molecule::label, "Gets the original label of the atom as given in the input file (C2, H4)").
            def("charge", &Molecule::charge, "Gets charge of atom").
            def("molecular_charge", &Molecule::molecular_charge, "Gets the molecular charge").
            def("extract_subsets", &Molecule::py_extract_subsets_1, "Returns copy of arg1 with arg2 fragments Real and arg3 fragments Ghost").
            def("extract_subsets", &Molecule::py_extract_subsets_2, "Returns copy of arg1 with arg2 fragments Real and arg3 fragment Ghost").
            def("extract_subsets", &Molecule::py_extract_subsets_3, "Returns copy of arg1 with arg2 fragment Real and arg3 fragments Ghost").
            def("extract_subsets", &Molecule::py_extract_subsets_4, "Returns copy of arg1 with arg2 fragment Real and arg3 fragment Ghost").
            def("extract_subsets", &Molecule::py_extract_subsets_5, "Returns copy of arg1 with arg2 fragments Real").
            def("extract_subsets", &Molecule::py_extract_subsets_6, "Returns copy of arg1 with arg2 fragment Real").
            def("activate_all_fragments", &Molecule::activate_all_fragments, "Sets all fragments in the molecule to be active").
            def("deactivate_all_fragments", &Molecule::deactivate_all_fragments, "Sets all fragments in the molecule to be inactive").
            def("set_active_fragments", &Molecule::set_active_fragments, "Sets the specified list arg2 of fragments to be Real").
            def("set_active_fragment", &Molecule::set_active_fragment, "Sets the specified fragment arg2 to be Real").
            def("set_ghost_fragments", &Molecule::set_ghost_fragments, "Sets the specified list arg2 of fragments to be Ghost").
            def("set_ghost_fragment", &Molecule::set_ghost_fragment, "Sets the specified fragment arg2 to be Ghost").
            def("atom_at_position", &Molecule::atom_at_position1, "Tests to see if an atom is at the position arg2 with a given tolerance arg3").
            def("print_out", &Molecule::print, "Prints the molecule in Cartesians in input units").
            def("print_out_in_bohr", &Molecule::print_in_bohr, "Prints the molecule in Cartesians in Bohr").
            def("print_out_in_angstrom", &Molecule::print_in_angstrom, "Prints the molecule in Cartesians in Angstroms").
            def("nuclear_repulsion_energy", &Molecule::nuclear_repulsion_energy, "Computes nuclear repulsion energy").
            def("find_point_group", &Molecule::find_point_group, "Finds computational molecular point group, user can override this with the symmetry keyword").
            def("reset_point_group", &Molecule::reset_point_group, "Overrides symmetry from outside the molecule string").
            def("set_point_group", &Molecule::set_point_group, "Sets the molecular point group to the point group object arg2").
            def("get_full_point_group", &Molecule::full_point_group, "Gets point group name such as C3v or S8").
            def("point_group", &Molecule::point_group, "Returns the current point group object").
            def("schoenflies_symbol", &Molecule::schoenflies_symbol, "Returns the Schoenflies symbol").
            def("form_symmetry_information", &Molecule::form_symmetry_information, "Uses the point group object obtain by calling point_group()").
            def("create_molecule_from_string", &Molecule::create_molecule_from_string, "Returns a new Molecule with member data from the geometry string arg1 in psi4 format").
            staticmethod("create_molecule_from_string").
            def("is_variable", &Molecule::is_variable, "Checks if variable arg2 is in the list, returns true if it is, and returns false if not").
            def("set_variable", &Molecule::set_variable, "Assigns the value arg3 to the variable arg2 in the list of geometry variables, then calls update_geometry()").
            def("get_variable", &Molecule::get_variable, "Checks if variable arg2 is in the list, sets it to val and returns true if it is, and returns false if not").
            def("update_geometry", &Molecule::update_geometry, "Reevaluates the geometry with current variable values, orientation directives, etc. Must be called after initial Molecule definition by string.").
            def("set_molecular_charge", &Molecule::set_molecular_charge, "Sets the molecular charge").
            def("set_multiplicity", &Molecule::set_multiplicity, "Sets the multiplicity (defined as 2Ms + 1)").
            def("set_basis_all_atoms", &Molecule::set_basis_all_atoms, "Sets basis set arg2 to all atoms").
            def("set_basis_by_symbol", &Molecule::set_basis_by_symbol, "Sets basis set arg3 to all atoms with symbol (e.g., H) arg2").
            def("set_basis_by_label", &Molecule::set_basis_by_label, "Sets basis set arg3 to all atoms with label (e.g., H4) arg2").
            def("set_basis_by_number", &Molecule::set_basis_by_number, "Sets basis set arg3 to atom number (1-indexed, incl. dummies) arg2").
            def("clone", &Molecule::clone, "Returns a new Molecule identical to arg1").
            def("geometry", &Molecule::geometry, "Gets the geometry as a (Natom X 3) matrix of coordinates (in Bohr)");

    class_<PetiteList, boost::shared_ptr<PetiteList>, boost::noncopyable>("PetiteList", "docstring", no_init).
            def("aotoso", &PetiteList::aotoso, "docstring").
            def("sotoao", &PetiteList::sotoao, "docstring").
            def("print", &PetiteList::print, "docstring");

    class_<BasisSetParser, boost::shared_ptr<BasisSetParser>, boost::noncopyable>("BasisSetParser", "docstring", no_init);
    class_<Gaussian94BasisSetParser, boost::shared_ptr<Gaussian94BasisSetParser>, bases<BasisSetParser> >("Gaussian94BasisSetParser", "docstring");

    typedef void (BasisSet::*basis_print_out)() const;
    class_<BasisSet, boost::shared_ptr<BasisSet>, boost::noncopyable>("BasisSet", "docstring", no_init).
            def("print_out", basis_print_out(&BasisSet::print), "docstring").
            def("print_detail_out", basis_print_out(&BasisSet::print_detail), "docstring").
            def("make_filename", &BasisSet::make_filename, "docstring").
            staticmethod("make_filename").
            def("construct", &BasisSet::construct, "docstring").
            staticmethod("construct").
            def("nbf", &BasisSet::nbf, "docstring").
            def("nao", &BasisSet::nao, "docstring").
            def("nprimitive", &BasisSet::nprimitive, "docstring").
            def("nshell", &BasisSet::nshell, "docstring").
            def("max_am", &BasisSet::max_am, "docstring").
            def("has_puream", &BasisSet::has_puream, "docstring").
            def(self + self);

    class_<SOBasisSet, boost::shared_ptr<SOBasisSet>, boost::noncopyable>("SOBasisSet", "docstring", no_init).
            def("petite_list", &SOBasisSet::petite_list, "docstring");

    class_<ExternalPotential, boost::shared_ptr<ExternalPotential>, boost::noncopyable>("ExternalPotential", "docstring").
            def("setName", &ExternalPotential::setName, "docstring").
            def("addCharge", &ExternalPotential::addCharge, "docstring").
            def("addBasis", &ExternalPotential::addBasis, "docstring").
            def("clear", &ExternalPotential::clear, "docstring").
            def("computePotentialMatrix", &ExternalPotential::computePotentialMatrix, "docstring").
            def("print_out", &ExternalPotential::py_print, "docstring");

    class_<DFChargeFitter, boost::shared_ptr<DFChargeFitter>, boost::noncopyable>("DFChargeFitter", "docstring").
            def("setPrimary", &DFChargeFitter::setPrimary, "docstring").
            def("setAuxiliary", &DFChargeFitter::setAuxiliary, "docstring").
            def("setD", &DFChargeFitter::setD, "docstring").
            def("d", &DFChargeFitter::d, "docstring").
            def("fit", &DFChargeFitter::fit, "docstring");

    class_<Wavefunction, boost::shared_ptr<Wavefunction>, boost::noncopyable>("Wavefunction", "docstring", no_init).
            def("nso", &Wavefunction::nso, "docstring").
            def("nmo", &Wavefunction::nmo, "docstring").
            def("nirrep", &Wavefunction::nirrep, "docstring").
            def("Ca", &Wavefunction::Ca, "docstring").
            def("Cb", &Wavefunction::Cb, "docstring").
            def("Fa", &Wavefunction::Fa, "docstring").
            def("Fb", &Wavefunction::Fb, "docstring").
            def("Da", &Wavefunction::Da, "docstring").
            def("Db", &Wavefunction::Db, "docstring").
            def("epsilon_a", &Wavefunction::epsilon_a, "docstring").
            def("epsilon_b", &Wavefunction::epsilon_b, "docstring").
            def("add_preiteration_callback", &Wavefunction::add_preiteration_callback, "docstring").
            def("add_postiteration_callback", &Wavefunction::add_postiteration_callback, "docstring").
            def("basisset", &Wavefunction::basisset, "docstring").
            def("sobasisset", &Wavefunction::sobasisset, "docstring").
            def("energy", &Wavefunction::reference_energy, "docstring").
            def("gradient", &Wavefunction::gradient, "docstring").
            def("frequencies", &Wavefunction::frequencies, "docstring").
            def("alpha_orbital_space", &Wavefunction::alpha_orbital_space, "docstring").
            def("beta_orbital_space", &Wavefunction::beta_orbital_space, "docstring").
            def("molecule", &Wavefunction::molecule, "docstring").
            def("doccpi", &Wavefunction::doccpi, "docstring").
            def("soccpi", &Wavefunction::soccpi, "docstring").
            def("nsopi", &Wavefunction::nsopi, "docstring").
            def("nmopi", &Wavefunction::nmopi, "docstring").
            def("nalphapi", &Wavefunction::nalphapi, "docstring").
            def("nbetapi", &Wavefunction::nbetapi, "docstring").
            def("frzcpi", &Wavefunction::frzcpi, "docstring").
            def("frzvpi", &Wavefunction::frzvpi, "docstring").
            def("nalpha", &Wavefunction::nalpha, "docstring").
            def("nbeta", &Wavefunction::nbeta, "docstring");

    class_<scf::HF, boost::shared_ptr<scf::HF>, bases<Wavefunction>, boost::noncopyable>("HF", "docstring", no_init);
    class_<scf::RHF, boost::shared_ptr<scf::RHF>, bases<scf::HF, Wavefunction> >("RHF", "docstring", no_init);

    class_<MoldenWriter, boost::shared_ptr<MoldenWriter> >("MoldenWriter", "docstring", no_init).
            def(init<boost::shared_ptr<Wavefunction> >()).
            def("write", &MoldenWriter::write, "docstring");

    class_<NBOWriter, boost::shared_ptr<NBOWriter> >("NBOWriter", "docstring", no_init).
            def(init<boost::shared_ptr<Wavefunction> >()).
            def("write", &NBOWriter::write, "docstring");

    class_<OperatorSymmetry, boost::shared_ptr<OperatorSymmetry> >("MultipoleSymmetry", "docstring", no_init).
            def(init<int, const boost::shared_ptr<Molecule>&,
                const boost::shared_ptr<IntegralFactory>&,
                const boost::shared_ptr<MatrixFactory>&>()).
            def("create_matrices", &OperatorSymmetry::create_matrices, "docstring");

    class_<CorrelationFactor, boost::shared_ptr<CorrelationFactor>, boost::noncopyable>("CorrelationFactor", "docstring", no_init).
            def(init<unsigned int>()).
            def(init<boost::shared_ptr<Vector>, boost::shared_ptr<Vector> >()).
            def("set_params", &CorrelationFactor::set_params, "docstring");
    class_<FittedSlaterCorrelationFactor, bases<CorrelationFactor>, boost::noncopyable>("FittedSlaterCorrelationFactor", "docstring", no_init).
            def(init<double>()).
            def("exponent", &FittedSlaterCorrelationFactor::exponent);

}
