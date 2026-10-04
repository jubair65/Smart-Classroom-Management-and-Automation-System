package com.uap.cse316.smartclassroom.ui.alerts

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.fragment.app.Fragment
import androidx.recyclerview.widget.LinearLayoutManager
import com.google.android.material.dialog.MaterialAlertDialogBuilder
import com.google.firebase.database.DataSnapshot
import com.google.firebase.database.DatabaseError
import com.google.firebase.database.ValueEventListener
import com.uap.cse316.smartclassroom.data.model.AlertItem
import com.uap.cse316.smartclassroom.databinding.FragmentAlertsBinding
import com.uap.cse316.smartclassroom.utils.FirebaseManager

class AlertsFragment : Fragment() {

    private var _binding: FragmentAlertsBinding? = null
    private val binding get() = _binding!!

    private lateinit var adapter: AlertAdapter
    private var alertsEventListener: ValueEventListener? = null

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentAlertsBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        adapter = AlertAdapter()
        binding.rvAlerts.layoutManager = LinearLayoutManager(requireContext())
        binding.rvAlerts.adapter = adapter

        binding.swipeRefreshAlerts.setOnRefreshListener {
            loadAlerts()
        }

        binding.btnDeleteAlerts.setOnClickListener {
            confirmClearAlerts()
        }

        loadAlerts()
    }

    private fun confirmClearAlerts() {
        MaterialAlertDialogBuilder(requireContext())
            .setTitle("Clear All Alerts?")
            .setMessage("This will permanently delete all security intrusion alert logs from Firebase.")
            .setPositiveButton("Clear All") { _, _ ->
                FirebaseManager.getAlertsReference().removeValue()
                    .addOnSuccessListener {
                        Toast.makeText(requireContext(), "All alerts cleared.", Toast.LENGTH_SHORT).show()
                    }
                    .addOnFailureListener { e ->
                        Toast.makeText(requireContext(), "Failed to clear: ${e.message}", Toast.LENGTH_SHORT).show()
                    }
            }
            .setNegativeButton("Cancel", null)
            .show()
    }

    private fun loadAlerts() {
        val alertsQuery = FirebaseManager.getAlertsReference().limitToLast(50)

        alertsEventListener?.let { alertsQuery.removeEventListener(it) }

        alertsEventListener = object : ValueEventListener {
            override fun onDataChange(snapshot: DataSnapshot) {
                if (!isAdded || _binding == null) return
                binding.swipeRefreshAlerts.isRefreshing = false

                val alerts = mutableListOf<AlertItem>()
                for (child in snapshot.children) {
                    val key = child.key ?: ""
                    if (key.startsWith("sample", ignoreCase = true)) continue

                    val alert = child.getValue(AlertItem::class.java)
                    if (alert != null) {
                        alert.id = key
                        alerts.add(alert)
                    }
                }

                if (alerts.isEmpty()) {
                    binding.rvAlerts.visibility = View.GONE
                    binding.emptyAlertsView.visibility = View.VISIBLE
                    binding.btnDeleteAlerts.visibility = View.GONE
                } else {
                    binding.rvAlerts.visibility = View.VISIBLE
                    binding.emptyAlertsView.visibility = View.GONE
                    binding.btnDeleteAlerts.visibility = View.VISIBLE
                    adapter.setAlerts(alerts)
                }
            }

            override fun onCancelled(error: DatabaseError) {
                if (!isAdded || _binding == null) return
                binding.swipeRefreshAlerts.isRefreshing = false
            }
        }

        alertsQuery.addValueEventListener(alertsEventListener as ValueEventListener)
    }

    override fun onDestroyView() {
        super.onDestroyView()
        alertsEventListener?.let {
            FirebaseManager.getAlertsReference().limitToLast(50).removeEventListener(it)
        }
        _binding = null
    }

}
